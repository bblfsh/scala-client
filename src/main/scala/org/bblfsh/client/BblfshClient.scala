package org.bblfsh.client

import org.bblfsh.client.libuast.Libuast

import gopkg.in.bblfsh.sdk.v1.protocol.generated.{Encoding, ParseRequest, 
                                                  ProtocolServiceGrpc, ParseResponse}
import gopkg.in.bblfsh.sdk.v1.uast.generated.Node

import io.grpc.ManagedChannelBuilder


class BblfshClient(host: String, port: Int, maxMsgSize: Int) {
  private val channel = ManagedChannelBuilder
    .forAddress(host, port)
    .usePlaintext(true)
    .maxInboundMessageSize(maxMsgSize)
    .build()
  private val stub = ProtocolServiceGrpc.blockingStub(channel)
  private val libuast = new Libuast

  def parse(name: String, content: String, lang: String = "", 
            encoding: Encoding = Encoding.UTF8) = {
    // assume content is already encoded in one of:
    // https://github.com/bblfsh/sdk/blob/master/protocol/protocol.go#L68
    val req = ParseRequest(filename = name,
                           content = content,
                           language = BblfshClient.normalizeLanguage(lang),
                           encoding = encoding)
    val parsed = stub.parse(req)
    parsed
  }

  def filter(node: Node, query: String): List[Node] = {
    libuast.filter(node, query)
  }

  // XXX remove
  def hasstartposition(node: Node): Boolean = {
    libuast.hasstartposition(node)
  }

  // XXX remove
  def hasendposition(node: Node): Boolean = {
    libuast.hasendposition(node)
  }

  // XXX remove
  def propertykeyat(node: Node, index: Int): String = {
    libuast.propertykeyat(node, index)
  }
  // XXX remove
  def propertyvalueat(node: Node, index: Int): String = {
    libuast.propertyvalueat(node, index)
  }

  // XXX remove
  def startoffset(node: Node): Int = {
    libuast.startoffset(node)
  }

  // XXX remove
  def startline(node: Node): Int = {
    libuast.startline(node)
  }

  // XXX remove
  def startcol(node: Node): Int = {
    libuast.startcol(node)
  }
  
  def endoffset(node: Node): Int = {
    libuast.endoffset(node)
  }

  // XXX remove
  def endline(node: Node): Int = {
    libuast.endline(node)
  }

  // XXX remove
  def endcol(node: Node): Int = {
    libuast.endcol(node)
  }
}

object BblfshClient {
  val DEFAULT_MAX_MSG_SIZE = 100 * 1024 * 1024

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
}

