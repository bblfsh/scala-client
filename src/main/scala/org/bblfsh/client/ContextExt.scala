package org.bblfsh.client

import java.nio.ByteBuffer

case class ContextExt(nativeContext: Long) {
  @native def filter()

  @native def root(): NodeExt //done
  @native def encode(n: NodeExt, format: Int): ByteBuffer //done
  @native def dispose() //done

  override def finalize(): Unit = {
    this.dispose()
  }
}
