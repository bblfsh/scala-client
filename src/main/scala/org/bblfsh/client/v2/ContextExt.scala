package org.bblfsh.client.v2

import java.nio.ByteBuffer

import org.bblfsh.client.v2.libuast.Libuast.{UastIter, UastIterExt}

/**
  * Represents Go-side constructed tree, result of Libuast.decode()
  *
  * This is equivalent of pyuast.ContextExt API
  */
case class ContextExt(nativeContext: Long) {
    import BblfshClient.uastBinary
    // @native def load(): JNode // TODO(bzz): clarify when it's needed VS just .root().load()
    @native def root(): NodeExt
    @native def filter(query: String): UastIterExt
    // encode using binary format
    @native def encode(n: NodeExt, fmt: Int): ByteBuffer
    def encode(n: NodeExt): ByteBuffer = {
      encode(n, uastBinary)
    }
    @native def dispose()
    override def finalize(): Unit = {
        this.dispose()
    }
}


/**
  * Represents JVM-side constructed tree
  *
  * This is equivalent of pyuast.Context API
  */
case class Context(nativeContext: Long) {
    import BblfshClient.uastBinary

    @native def root(): JNode
    @native def filter(query: String, node: JNode): UastIter
    @native def encode(n: JNode, fmt: Int): ByteBuffer
    // encode using binary format
    def encode(n: JNode): ByteBuffer = {
      encode(n, uastBinary)
    }
    @native def dispose()
    override def finalize(): Unit = {
      this.dispose()
    }
}
object Context {
    @native def create(): Long
    def apply(): Context = new Context(create())
}
