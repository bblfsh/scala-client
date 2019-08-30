package org.bblfsh.client.v2

import java.nio.ByteBuffer

import com.google.protobuf.ByteString
import gopkg.in.bblfsh.sdk.v2.protocol.driver._
import io.grpc.ManagedChannelBuilder
import org.bblfsh.client.v2.libuast.Libuast


class BblfshClient(host: String, port: Int, maxMsgSize: Int) {
  private val DEFAULT_TIMEOUT_SEC = 5

  private val channel = ManagedChannelBuilder
    .forAddress(host, port)
    .usePlaintext(true)
    .maxInboundMessageSize(maxMsgSize)
    .build()

  private val stub = DriverGrpc.blockingStub(channel)
  private val stubInfo = DriverHostGrpc.blockingStub(channel)

  /**
    * Parses file with a given name and content using
    * the provided timeout.
    *
    * @param name    file name
    * @param content file content
    * @param lang    (optional) language to parse, default auto-detect \w enry
    * @param timeout (disabled) bblfsh request timeout, seconds
    *                Right now this does not have any effect in v2.
    * @param mode    (optional) mode to parse, default to bblfshd 'default' mode
    * @return UAST in parse response.
    */
  def parseWithOptions(
    name: String,
    content: String,
    lang: String,
    timeout: Long,
    mode: Mode
  ): ParseResponse = {
    // TODO(#100): make timeout work in v2 again
    val req = ParseRequest(
      filename = name,
      content = content,
      language = lang,
      mode = mode
    )
    stub.parse(req)
  }

  /**
    * Parses the given file name and content.
    *
    * A default timeout of 5 seconds will be applied, same as was done by the server
    * before https://github.com/bblfsh/bblfshd/blob/83166ea0087bfe20c24fc471309f70f422383198/daemon/pool.go#L191
    *
    * @param name    file name
    * @param content file content
    * @param lang    (optional) language to parse, default auto-detect \w enry
    * @return UAST in parse response.
    */
  def parse(
    name: String,
    content: String,
    lang: String = ""
  ): ParseResponse = parseWithOptions(name, content, lang, DEFAULT_TIMEOUT_SEC, Mode.DEFAULT_MODE)

  def parse(
    name: String,
    content: String,
    mode: Mode
  ): ParseResponse = parseWithOptions(name, content, "", DEFAULT_TIMEOUT_SEC, mode)

  /**
    * Parses the given file name and content,
    * setting a bblfsh request timeout.
    *
    * Since v1.11, this API exposes the timeout.
    *
    * @param name    file name
    * @param content file content
    * @param timeout bblfsh request timeout, seconds
    * @param lang    (optional) language to parse, default auto-detect \w enry
    * @return
    */
  def parseWithTimeout(
    name: String,
    content: String,
    timeout: Long,
    lang: String = ""
  ): ParseResponse = parseWithOptions(name, content, lang, timeout, Mode.DEFAULT_MODE)

  def supportedLanguages(): SupportedLanguagesResponse = {
    val req = SupportedLanguagesRequest()
    stubInfo.supportedLanguages(req)
  }

  def version(): VersionResponse = {
    val req = VersionRequest()
    stubInfo.serverVersion(req)
  }

  def close(): Unit = {
    channel.shutdownNow()
  }
}

object BblfshClient {
  val DEFAULT_MAX_MSG_SIZE = 100 * 1024 * 1024 // bytes

  private val libuast = new Libuast
  private val orders = libuast.getTreeOrders
  private val formats = libuast.getUastFormats

  abstract class UastFormat(val toInt: Int)
  abstract class TreeOrder(val toInt: Int)

  // Lift tree order as constants
  case object UastBinary extends UastFormat(formats.uastBinary)
  case object UastYaml extends UastFormat(formats.uastYaml)

  // Lift orders from libuast as types
  case object AnyOrder extends TreeOrder(orders.anyOrder)
  case object PreOrder extends TreeOrder(orders.preOrder)
  case object PostOrder extends TreeOrder(orders.postOrder)
  case object LevelOrder extends TreeOrder(orders.levelOrder)
  case object ChildrenOrder extends TreeOrder(orders.childrenOrder)
  case object PositionOrder extends TreeOrder(orders.positionOrder)

  /** Creates a BblfshClient with default parameters */
  def apply(
    host: String, port: Int,
    maxMsgSize: Int = DEFAULT_MAX_MSG_SIZE
  ): BblfshClient = new BblfshClient(host, port, maxMsgSize)

  /**
    * Decodes bytes from wired format of bblfsh protocol.v2.
    * Requires a buffer in Direct mode, and the format
    * to decode from
    *
    * Since v2.
    */
  def decode(buf: ByteBuffer, fmt: UastFormat): ContextExt = Libuast.synchronized {
    if (!buf.isDirect()) {
      throw new RuntimeException("Only directly-allocated buffer decoding is supported.")
    }
    libuast.decode(buf, fmt)
  }

  /**
    * Decodes bytes from wired binary format of bblfsh protocol.v2.
    * Requires a buffer in Direct mode
    *
    * Since v2.
    */
  def decode(buf: ByteBuffer): ContextExt = Libuast.synchronized {
    decode(buf, UastBinary)
  }

  /** Enables API: resp.uast.decode() */
  implicit class UastMethods(val buf: ByteString) {
    /**
      * Decodes bytes from wire format of bblfsh protocol.v2.
      *
      * Always copies memory to a new buffer in Direct mode,
      * to be able to pass it to JNI.
      */
    def decode(fmt: UastFormat): ContextExt = {
      val bufDirectCopy = ByteBuffer.allocateDirect(buf.size)
      buf.copyTo(bufDirectCopy)
      val result = BblfshClient.decode(bufDirectCopy, fmt)
      // Sometimes the direct buffer can take a lot to deallocate,
      // causing Out of Memory, because it is not allocated in
      // in the JVM heap and will only be deallocated them when
      // JVM does not have more space in its heap
      // This line alleviates the problem
      // Problem can be tested if we remove it and do a
      // loop decoding a file
      System.gc()
      result
    }

    /**
      * Decodes in binary format
      */
    def decode(): ContextExt = {
      decode(UastBinary)
    }
  }

  /** Enables API: resp.get() */
  implicit class ResponseMethods(val resp: ParseResponse) {
    /** Gets the root decoding the tree in binary format */
    def get(fmt: UastFormat): JNode = {
      val ctx = resp.uast.decode(fmt)
      val node = ctx.root().load()
      ctx.dispose()
      node
    }

    /** Gets the root node decoding the tree in binary format */
    def get(): JNode = {
      get(UastBinary)
    }
  }

  /** Enables API: client.filter and client.iterator for client an instance of BblfshClient */
  implicit class BblfshClientMethods(val client: BblfshClient) {
    def filter(node: NodeExt, query: String) = BblfshClient.filter(node, query)
    def filter(node: JNode, query: String) = BblfshClient.filter(node, query)
    def iterator(node: NodeExt, treeOrder: TreeOrder) = BblfshClient.iterator(node, treeOrder)
    def iterator(node: JNode, treeOrder: TreeOrder) = BblfshClient.iterator(node, treeOrder)
  }

  /** Factory method for iterator over an external/native node */
  def iterator(node: NodeExt, treeOrder: TreeOrder): Libuast.UastIterExt = {
    Libuast.UastIterExt(node, treeOrder)
  }

  /** Factory method for iterator over an managed node */
  def iterator(node: JNode, treeOrder: TreeOrder): Libuast.UastIter = {
    Libuast.UastIter(node, treeOrder)
  }

  /** Factory method for iterator over an native node, filtered by XPath query */
  def filter(node: NodeExt, query: String):  Libuast.UastIterExt = Libuast.synchronized {
    node.filter(query)
  }

  /** Factory method for iterator over an managed node, filtered by XPath query */
  def filter(node: JNode, query: String):  Libuast.UastIter = Libuast.synchronized {
    val ctx = Context()
    ctx.filter(query, node)
    // do not dispose the context, iterator steals it
  }

}
