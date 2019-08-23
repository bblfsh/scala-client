package org.bblfsh.client.v2
	
import java.nio.ByteBuffer

import org.bblfsh.client.v2.libuast.Libuast.{UastIter, UastIterExt}

/**
  * Represents Go-side constructed tree, result of Libuast.decode()
  *
  * This is equivalent of pyuast.ContextExt API
  */
case class ContextExt(nativeContext: Long) {
    // @native def load(): JNode // TODO(bzz): clarify when it's needed VS just .root().load()
    @native def root(): NodeExt
    @native def filter(query: String): UastIterExt
    @native def encode(n: NodeExt): ByteBuffer
    @native private def dispose()
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
    @native def root(): JNode
    @native def filter(query: String, node: JNode): UastIter
    @native def encode(n: JNode): ByteBuffer
    @native private def dispose()
    override def finalize(): Unit = {
      this.dispose
    } 
}
object Context {
    @native def create(): Long
    def apply(): Context = new Context(create())
}
