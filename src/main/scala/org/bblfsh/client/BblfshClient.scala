package org.bblfsh.client

import com.google.protobuf.duration.Duration
import org.bblfsh.client.libuast.Libuast
import gopkg.in.bblfsh.sdk.v1.protocol.generated.{Encoding, NativeParseRequest, NativeParseResponse, ParseRequest, ParseResponse, ProtocolServiceGrpc, SupportedLanguagesRequest, SupportedLanguagesResponse, VersionRequest, VersionResponse}
import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import io.grpc.ManagedChannelBuilder


class BblfshClient(host: String, port: Int, maxMsgSize: Int) {

  private val DEFAULT_TIMEOUT_SEC = 5

  private val channel = ManagedChannelBuilder
    .forAddress(host, port)
    .usePlaintext(true)
    .maxInboundMessageSize(maxMsgSize)
    .build()

  private val stub = ProtocolServiceGrpc.blockingStub(channel)

  /**
    * Parses file with a given name and content using an encoding
    * and a timeout provided.
    *
    * @param name file name
    * @param content file content
    * @param lang (optional) language to parse, default auto-detect \w enry
    * @param timeout bblfsh request timeout, seconds
    * @param encoding (optional) encoding of the content, default UTF8
    * @return UAST in parse response.
    */
  def parseWithOptions(
    name: String,
    content: String,
    lang: String,
    timeout: Long,
    encoding: Encoding
  ): ParseResponse = {
    // assume content is already encoded in one of:
    // https://github.com/bblfsh/sdk/blob/master/protocol/protocol.go#L68
    val req = ParseRequest(
      filename = name,
      content = content,
      language = BblfshClient.normalizeLanguage(lang),
      encoding = encoding,
      timeout = Option(Duration(seconds = timeout))
    )
    stub.parse(req)
  }

  /**
    * Parses the given file name and content using the encoding.
    *
    * A default timeout of 5 seconds will be applied, same as was done by the server
    * before https://github.com/bblfsh/bblfshd/blob/83166ea0087bfe20c24fc471309f70f422383198/daemon/pool.go#L191
    *
    * @param name file name
    * @param content file content
    * @param lang (optional) language to parse, default auto-detect \w enry
    * @param encoding (optional) encoding of the content, default UTF8
    * @return UAST in parse response.
    */
  def parse(
    name: String,
    content: String,
    lang: String = "",
    encoding: Encoding = Encoding.UTF8
  ): ParseResponse = parseWithOptions(name, content, lang, DEFAULT_TIMEOUT_SEC, encoding)

  /**
    * Parses the given file name and content,
    * setting a bblfsh request timeout.
    *
    * Since v1.11, this API exposes the timeout.
    *
    * @param name file name
    * @param content file content
    * @param timeout bblfsh request timeout, seconds
    * @param lang (optional) language to parse, default auto-detect \w enry
    * @return
    */
  def parseWithTimeout(
    name: String,
    content: String,
    timeout: Long,
    lang: String = ""
  ): ParseResponse = parseWithOptions(name, content, lang, timeout, Encoding.UTF8)

  def nativeParse(
    name: String,
    content: String,
    lang: String = "",
    encoding: Encoding = Encoding.UTF8
  ): NativeParseResponse = {
    // assume content is already encoded in one of:
    // https://github.com/bblfsh/sdk/blob/master/protocol/protocol.go#L68
    val req = NativeParseRequest(filename = name,
      content = content,
      language = BblfshClient.normalizeLanguage(lang),
      encoding = encoding)
    stub.nativeParse(req)
  }

  def supportedLanguages(): SupportedLanguagesResponse = {
    val req = SupportedLanguagesRequest()
    stub.supportedLanguages(req)
  }

  def version(): VersionResponse = {
    val req = VersionRequest()
    stub.version(req)
  }

  def close(): Unit = {
    channel.shutdownNow()
  }

  /**
   * Proxy for Bblfsh.filter / Node.filter, provided for backward compatibility.
   */
  def filter(node: Node, query: String): List[Node] = {
    BblfshClient.filter(node, query)
  }

  def filterBool(node: Node, query: String): Boolean = {
    BblfshClient.filterBool(node, query)
  }

  def filterNumber(node: Node, query: String): Double = {
    BblfshClient.filterNumber(node, query)
  }

  def filterString(node: Node, query: String): String = {
    BblfshClient.filterString(node, query)
  }
}

object BblfshClient {
  val DEFAULT_MAX_MSG_SIZE = 100 * 1024 * 1024

  val PreOrder      = 0
  val PostOrder     = 1
  val LevelOrder    = 2
  val PositionOrder = 3

  private val libuast = new Libuast

  def apply(host: String, port: Int,
            maxMsgSize: Int = DEFAULT_MAX_MSG_SIZE): BblfshClient =
    new BblfshClient(host, port, maxMsgSize)


  def normalizeLanguage(lang: String): String = {
    if (lang == null) {
      return ""
    }

    lang
      .toLowerCase
      .replace(" ", "-")
      .replace("+", "p")
      .replace("#", "sharp")
  }

  def filter(node: Node, query: String): List[Node] = Libuast.synchronized {
    libuast.filter(node, query)
  }

  def filterBool(node: Node, query: String): Boolean = Libuast.synchronized {
    libuast.filterBool(node, query)
  }

  def filterNumber(node: Node, query: String): Double = Libuast.synchronized {
    libuast.filterNumber(node, query)
  }

  def filterString(node: Node, query: String): String = Libuast.synchronized {
    libuast.filterString(node, query)
  }

  def iterator(node: Node, treeOrder: Int): Libuast.UastIterator = {
    libuast.iterator(node, treeOrder)
  }

  implicit class NodeMethods(val node: Node) {
    def filter(query: String): List[Node] = {
      BblfshClient.filter(node, query)
    }

    def filterBool(query: String): Boolean = {
      BblfshClient.filterBool(node, query)
    }

    def filterNumber(query: String): Double = {
      BblfshClient.filterNumber(node, query)
    }

    def filterString(query: String): String = {
      BblfshClient.filterString(node, query)
    }
  }
}

