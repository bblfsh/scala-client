package org.bblfsh.client

import java.nio.ByteBuffer

import com.google.protobuf.ByteString
import gopkg.in.bblfsh.sdk.v2.protocol.driver.{DriverGrpc, Mode, ParseRequest, ParseResponse}
import gopkg.in.bblfsh.sdk.v2.uast.nodes.nodes.Node
import org.bblfsh.client.libuast.Libuast
import io.grpc.ManagedChannelBuilder


class BblfshClient(host: String, port: Int, maxMsgSize: Int) {
  private val channel = ManagedChannelBuilder
    .forAddress(host, port)
    .usePlaintext(true)
    .maxInboundMessageSize(maxMsgSize)
    .build()
  private val stub = DriverGrpc.blockingStub(channel)

  def parse(filename: String, content: String, lang: String = "", mode: Mode = Mode.DEFAULT_MODE): ParseResponse = {
    val req = ParseRequest(filename = filename,
      content = content,
      language = BblfshClient.normalizeLanguage(lang),
      mode = mode)
    stub.parse(req)
  }

  //TODO(bzz) use sdk.v1
//  def supportedLanguages(): SupportedLanguagesResponse = {
//    val req = SupportedLanguagesRequest()
//    stub.supportedLanguages(req)
//  }
//
//  def version(): VersionResponse = {
//    val req = VersionRequest()
//    stub.version(req)
//  }

  def close(): Unit = {
    channel.shutdownNow()
  }

  /**
   * Proxy for Bblfsh.filter / Node.filter, provided for backward compatibility.
   */
  def filter(node: NodeExt, query: String): List[NodeExt] = {
    BblfshClient.filter(node, query)
  }

  def filterBool(node: NodeExt, query: String): Boolean = {
    BblfshClient.filterBool(node, query)
  }

  def filterNumber(node: NodeExt, query: String): Double = {
    BblfshClient.filterNumber(node, query)
  }

  def filterString(node: NodeExt, query: String): String = {
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

  private def decode(buf: ByteBuffer): ContextExt = Libuast.synchronized {
    if (!buf.isDirect()) {
      throw new RuntimeException("Only directly-allocated buffer decoding is supported.")
    }
    libuast.decode(buf)
  }

  def filter(node: NodeExt, query: String): List[NodeExt] = Libuast.synchronized {
    libuast.filter(node, query)
  }

  def filterBool(node: NodeExt, query: String): Boolean = Libuast.synchronized {
    libuast.filterBool(node, query)
  }

  def filterNumber(node: NodeExt, query: String): Double = Libuast.synchronized {
    libuast.filterNumber(node, query)
  }

  def filterString(node: NodeExt, query: String): String = Libuast.synchronized {
    libuast.filterString(node, query)
  }

  def iterator(node: NodeExt, treeOrder: Int): Libuast.UastIterator = {
    libuast.iterator(node, treeOrder)
  }

  implicit class NodeMethods(val node: NodeExt) {
    def filter(query: String): List[NodeExt] = {
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

  // Enables API: resp.uast.decode()
  implicit class UastMethods(val buf: ByteString) {
    def decode(): ContextExt = {
      val bufDirectCopy = ByteBuffer.allocateDirect(buf.size)
      buf.copyTo(bufDirectCopy)
      BblfshClient.decode(bufDirectCopy)
    }
  }

}

