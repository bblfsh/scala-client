package org.bblfsh.client

import java.nio.ByteBuffer

import gopkg.in.bblfsh.sdk.v2.protocol.driver.ParseResponse
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FlatSpec, Matchers}

import scala.io.Source

class BblfshClientParseTest extends FlatSpec
  with BeforeAndAfter
  with BeforeAndAfterAll
  with Matchers {

  val client = BblfshClient("127.0.0.1", 9432)
  val fileName = "src/test/resources/SampleJavaFile.java"
  val fileContent = Source.fromFile(fileName).getLines.mkString
  var resp: ParseResponse = _

  import BblfshClient._ //enables uast.* methods

  before {
    resp = client.parse(fileName, fileContent)
  }

  override def afterAll {
    client.close()
  }

  "Parsed UAST for existing .java file" should "not be empty" in {
    assert(resp != null)
    assert(resp.errors.isEmpty)
  }

  "Decoded UAST after parsing" should "not be NULL" in {
    val uast = resp.uast.decode()

    assert(uast != null)
    assert(uast.nativeContex != 0)

    println(uast)
    uast.dispose()
  }

  "Decoded UAST's RootNode" should "not be NULL" in {
    val uast = resp.uast.decode()
    val rootNode: NodeExt = uast.root()
    println(rootNode)

    rootNode should not be null
    rootNode.ctx should not be (0)
    rootNode.handle should not be (0)

    uast.dispose()
  }

  "Encoding back the Root node of decoded UAST" should "produce same bytes" in {
    val uast = resp.uast.decode()
    val rootNode: NodeExt = uast.root()
    println(s"Root node: $rootNode")

    val encodedBytes: ByteBuffer = uast.encode(rootNode, 0)

    encodedBytes.capacity should be (resp.uast.asReadOnlyByteBuffer.capacity)
    encodedBytes should be equals resp.uast.asReadOnlyByteBuffer

    println(resp.uast.asReadOnlyByteBuffer)
    println(encodedBytes)
  }

  "Loading data from Go to JVM" should "bring native object to memory" in {
    val uast = resp.uast.decode()
    val rootNode: NodeExt = uast.root()

    println(s"Loading $rootNode")
    val v = rootNode.load()
    v should not be Nil
    v should not be empty
  }




  //  test("Get the children") {
  //    val children = rootNode.children
  //    assert(children.length == 2)
  //  }
  //
  //  test("Get the token") {
  //    assert(rootNode.children.head.token == "package")
  //  }
  //
  //  test("Get the properties") {
  //    val properties = rootNode.children.head.properties
  //    assert(properties.size == 1)
  //    assert(properties("internalRole") == "package")
  //  }
  //
  //  test("Simple XPath query called on client object andinstance") {
  //    var filtered = client.filter(rootNode, "//QualifiedName[@roleExpression]")
  //    var filtered2 = BblfshClient.filter(rootNode, "//QualifiedName[@roleExpression]")
  //    assert(filtered.length == 3)
  //    assert(filtered == filtered2)
  //  }
  //
  //  test("Simple XPath query calling filter on the Node") {
  //    rootNode.filter("//QualifiedName[@roleExpression]")
  //  }
  //
  //  test("XPath query with wrong type") {
  //    assertThrows[Exception]{ rootNode.filter("boolean(1)") }
  //  }
  //
  //  test("XPath query returning a boolean value") {
  //    val resTrue = rootNode.filterBool("boolean(1)")
  //    assert(resTrue)
  //    val resFalse = rootNode.filterBool("boolean(0)")
  //    assert(!resFalse)
  //  }
  //
  //  test("XPath query returning a number value") {
  //    val res = rootNode.filterNumber("count(//*)")
  //    assert(res == 28.0)
  //  }
  //
  //  test("XPath query returning a string value") {
  //    val res = rootNode.filterString("name(//*[1])")
  //    assert(res == "CompilationUnit")
  //  }
  //
  //  test("XPath query with threads") {
  //    val th = new Thread(new Runnable {
  //      def run() {
  //        var filtered = rootNode.filter("//QualifiedName[@roleExpression]")
  //        assert(filtered.length == 3)
  //      }
  //    })
  //    th.start
  //
  //    th.synchronized {
  //      th.wait
  //    }
  //  }
  //
  //  test("XPath filter properties") {
  //    val filtered = rootNode.filter("//*[@internalRole='types']");
  //    assert(filtered.length == 1)
  //    val filteredNeg = rootNode.filter("//*[@internalRole='foo']");
  //    assert(filteredNeg.length == 0)
  //  }
  //
  //  test("Xpath filter StartOffset") {
  //    val filtered = rootNode.filter("//*[@startOffset='24']");
  //    assert(filtered.length == 1)
  //    val filteredNeg = rootNode.filter("//*[@startOffset='44']");
  //    assert(filteredNeg.length == 0)
  //  }
  //
  //  test("Xpath filter StartLine") {
  //    val filtered = rootNode.filter("//*[@startLine='1']");
  //    assert(filtered.length == 28)
  //    val filteredNeg = rootNode.filter("//*[@startLine='100']");
  //    assert(filteredNeg.length == 0)
  //  }
  //
  //  test("Xpath filter StartCol") {
  //    val filtered = rootNode.filter("//*[@startCol='25']");
  //    assert(filtered.length == 1)
  //    val filteredNeg = rootNode.filter("//*[@startCol='999']");
  //    assert(filteredNeg.length == 0)
  //  }
  //
  //  test("Xpath filter EndOffset") {
  //    val filtered = rootNode.filter("//*[@endOffset='44']");
  //    assert(filtered.length == 1)
  //    val filteredNeg = rootNode.filter("//*[@endOffset='999']");
  //    assert(filteredNeg.length == 0)
  //  }
  //
  //  test("Xpath filter EndLine") {
  //    val filtered = rootNode.filter("//*[@endLine='1']");
  //    assert(filtered.length == 26)
  //    val filteredNeg = rootNode.filter("//*[@endLine='100']");
  //    assert(filteredNeg.length == 0)
  //  }
  //
  //  test("Xpath filter EndCol") {
  //    val filtered = rootNode.filter("//*[@endCol='45']");
  //    assert(filtered.length == 1)
  //    val filteredNeg = rootNode.filter("//*[@endCol='999']");
  //    assert(filteredNeg.length == 0)
  //  }
  //
  //  test("Get the start position") {
  //    val childWithPos = rootNode.children(1)
  //    val startPos = childWithPos.startPosition
  //    assert(!startPos.isEmpty)
  //    assert(startPos.get.offset == 24)
  //    assert(startPos.get.line == 1)
  //    assert(startPos.get.col == 25)
  //  }
  //
  //  test("Get the end position") {
  //    val childWithPos = rootNode.children(1).children.head
  //    val endPos = childWithPos.endPosition
  //    assert(!endPos.isEmpty)
  //    assert(endPos.get.offset == 44)
  //    assert(endPos.get.line == 1)
  //    assert(endPos.get.col == 45)
  //  }


}