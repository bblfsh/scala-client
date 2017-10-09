package org.bblfsh.client

import org.bblfsh.client.libuast.Libuast

import gopkg.in.bblfsh.sdk.v1.protocol.generated.{Encoding, ParseRequest, 
                                                  ProtocolServiceGrpc}
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

  // XXX make private for testing
  def readfield(node: Node, field: String): String = { 
    // XXX move to constructor
    libuast.readfield(node, field)
  }

  // XXX make private for testing
  def readlen(node: Node, field: String): Int = {
    // XXX move to constructor
    libuast.readlen(node, field)
  } 

  // XXX make private for testing
  def InternalType(node: Node): String = {
    libuast.InternalType(node)
  }

  // XXX make private for testing
  def Token(node: Node): String = {
    libuast.Token(node)
  }

  // XXX make private for testing
  def ChildrenSize(node: Node): Int = {
    libuast.ChildrenSize(node)
  }

  // XXX make private for testing
  def RolesSize(node: Node): Int = {
    libuast.RolesSize(node)
  }

  // XXX make private for testing
  def ChildAt(node: Node, index: Int): Node = {
    libuast.ChildAt(node, index)
  }

  // XXX make private for testing
  def PropertiesSize(node: Node): Int = {
    libuast.PropertiesSize(node)
  }

  // XXX make private for testing
  def PropertyAt(node: Node, index: Int): String = {
    libuast.PropertyAt(node, index)
  }

  // XXX make private for testing
  def RoleAt(node: Node, index: Int): Int = {
    libuast.RoleAt(node, index)
  }

  def filter(node: Node, query: String): List[Node] = {
    libuast.filter(node, query)
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

