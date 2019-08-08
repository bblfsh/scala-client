package org.bblfsh.client.v2

import java.nio.ByteBuffer

import com.google.protobuf.ByteString


/**
  * Tests for high-level API wrappers around decode/load,
  * checking result equivalence to the low-level counterpart.
  */
class BblfshClientUastApiTest extends BblfshClientBaseTest {

  import BblfshClient._ // enables uast.* methods

  override val fileName = "src/test/resources/Tiny.java"

  // Depends on having bblfshd JavaScript and Java drivers
  "SupportedLanguages" should "include aliases" in {
    val resp = client.supportedLanguages()

    val supportedLangs = resp.languages.flatMap(x => Seq(x.language) ++ x.aliases)

    supportedLangs.length should be > resp.languages.length
    resp.languages.size should be >= 2
  }

  "Parse + decode + load UAST" should "result in new JNode"  in {
    val node1 = resp.uast.decode.root.load()

    val node2 = resp.get()

    node2 should not be (null)
    node2 shouldBe a [JNode]
    node1 should equal(node2)
  }

  "Parse + get binary" should "result in ByteBuffer" in {
    resp.uast shouldBe a [ByteString]
  }

  "Decode binary to JVM memory" should "result in new JNode" in {
    val bytes: Array[Byte] = resp.uast.toByteArray
    val node1 = resp.uast.decode.root.load()

    val node2 = JNode.parseFrom(bytes)
    node2 should not be (null)
    node2 shouldBe a [JNode]
    node1 should equal(node2)

    val node3 = JNode.parseFrom(resp.uast.asReadOnlyByteBuffer())
    node3 should not be (null)
    node3 shouldBe a [JNode]
    node1 should equal(node3)
  }

  "Encode JNode to the binary" should "result in bytes" in {
    val node: JNode = JArray(
      JObject(
        "k1" -> JString("v1")
      ),
      JString("test")
    )

    val ctx = Context()
    val bytes1 = ctx.encode(node)
    ctx.dispose()

    val bytes2 = node.toByteBuffer
    bytes2 should not be (null)
    bytes2 shouldBe a[ByteBuffer]
    bytes2 should equal(bytes1)

    val bytes3 = node.toByteArray
    bytes3 should not be (null)
    bytes3 shouldBe a[Array[_]]
    ByteBuffer.wrap(bytes3) should equal(bytes1)
  }

  "XPath query" should "filter native UAST" in {
    val uast = resp.uast.decode().root()
    val it = BblfshClient.filter(uast, "//uast:Position")

    it.hasNext should be(true)
    it.toList should have size(8)

    it.close()
    it.hasNext should be(false)
  }

}
