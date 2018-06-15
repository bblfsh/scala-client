package org.bblfsh.client

import org.bblfsh.client.libuast.Libuast

import gopkg.in.bblfsh.sdk.v1.protocol.generated.{Encoding, ProtocolServiceGrpc,
                                                  ParseRequest, ParseResponse,
                                                  NativeParseRequest, NativeParseResponse,
                                                  SupportedLanguagesRequest, SupportedLanguagesResponse,
                                                  VersionRequest, VersionResponse}
import gopkg.in.bblfsh.sdk.v1.uast.generated.Node

import io.grpc.ManagedChannelBuilder


class BblfshClient(host: String, port: Int, maxMsgSize: Int) {
  private val channel = ManagedChannelBuilder
    .forAddress(host, port)
    .usePlaintext(true)
    .maxInboundMessageSize(maxMsgSize)
    .build()
  private val stub = ProtocolServiceGrpc.blockingStub(channel)

  def parse(name: String, content: String, lang: String = "", 
            encoding: Encoding = Encoding.UTF8): ParseResponse = {
    // assume content is already encoded in one of:
    // https://github.com/bblfsh/sdk/blob/master/protocol/protocol.go#L68
    val req = ParseRequest(filename = name,
                           content = content,
                           language = BblfshClient.normalizeLanguage(lang),
                           encoding = encoding)
    stub.parse(req)
  }

  def nativeParse(name: String, content: String, lang: String = "",
                  encoding: Encoding = Encoding.UTF8): NativeParseResponse = {
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

