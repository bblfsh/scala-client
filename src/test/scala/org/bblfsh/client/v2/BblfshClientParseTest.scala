package org.bblfsh.client.v2

import java.nio.ByteBuffer

import gopkg.in.bblfsh.sdk.v2.protocol.driver.ParseResponse
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FlatSpec, Matchers}

import scala.io.Source

class BblfshClientParseTest extends FlatSpec
  with BeforeAndAfter
  with BeforeAndAfterAll
  with Matchers {

  val client = BblfshClient("0.0.0.0", 9432)
  val fileName = "src/test/resources/SampleJavaFile.java"
  val fileContent = Source.fromFile(fileName).getLines.mkString("\n")
  var resp: ParseResponse = _

  import BblfshClient._ // enables uast.* methods

  before {
    resp = client.parse(fileName, fileContent)
  }

  override def afterAll {
    client.close()
  }

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
    val rootNode: Node = uast.root()
    println(rootNode.getClass)

    rootNode should not be null
    rootNode.ctx should not be (0)
    rootNode.handle should not be (0)

    uast.dispose()
  }

  "Encoding back the RootNode of decoded UAST" should "produce same bytes" in {
    val uast = resp.uast.decode()
    val rootNode: Node = uast.root()
    println(s"Root node: $rootNode")

    val encodedBytes: ByteBuffer = uast.encode(rootNode, 0)

    encodedBytes.capacity should be (resp.uast.asReadOnlyByteBuffer.capacity)
    encodedBytes shouldEqual resp.uast.asReadOnlyByteBuffer

    println(resp.uast.asReadOnlyByteBuffer)
    println(encodedBytes)
  }

}
