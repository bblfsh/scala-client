package org.bblfsh.client.v2

import java.nio.ByteBuffer


class BblfshClientParseTest extends BblfshClientBaseTest {

  import BblfshClient._ // enables uast.* methods

  "Parsed UAST for .java file" should "not be empty" in {
    assert(resp != null)
    assert(resp.errors.isEmpty)
  }

  "Decoded UAST after parsing" should "not be NULL" in {
    val uast = resp.uast.decode()

    assert(uast != null)
    assert(uast.nativeContext != 0)

    println(uast)
    uast.dispose()
  }

  "Decoded UAST RootNode" should "not be NULL" in {
    val uast = resp.uast.decode()
    val rootNode: NodeExt = uast.root()
    println(rootNode.getClass)

    rootNode should not be null
    rootNode.ctx should not be (0)
    rootNode.handle should not be (0)

    uast.dispose()
  }

  "Encoding back the RootNode of decoded UAST" should "produce same bytes" in {
    val uastCtx: ContextExt = resp.uast.decode()
    val rootNode: NodeExt = uastCtx.root()
    println(s"Root node: $rootNode")

    val encodedBytes: ByteBuffer = uastCtx.encode(rootNode)

    encodedBytes.capacity should be(resp.uast.asReadOnlyByteBuffer.capacity)
    encodedBytes shouldEqual resp.uast.asReadOnlyByteBuffer

    println(resp.uast.asReadOnlyByteBuffer)
    println(encodedBytes)
  }

}
