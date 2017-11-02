package org.bblfsh.client

import org.bblfsh.client.BblfshClient._

import gopkg.in.bblfsh.sdk.v1.protocol.generated.ParseResponse
import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import org.scalatest.FunSuite
import org.scalatest.BeforeAndAfter

import java.io.File
import scala.io.Source

class BblfshClientTest extends FunSuite with BeforeAndAfter {

  val client = BblfshClient("0.0.0.0", 9432)
  val fileName = "src/test/resources/SampleJavaFile.java"
  val fileContent = Source.fromFile(fileName) .getLines.mkString
  var resp: ParseResponse = _
  var rootNode: Node = _

  before {
    resp = client.parse(fileName, fileContent)
    
    if (resp.uast.isDefined) {
      rootNode = resp.uast.get
    }
  }

  test("Parse UAST for existing .java file") {
    assert(resp.errors.isEmpty)
    assert(resp.uast.isDefined)
  }

  test("Get the internalType") {
    assert(rootNode.internalType == "CompilationUnit")
  }

  test("Get the children") {
    val children = rootNode.children
    assert(children.length == 2)
  }

  test("Get the token") {
    assert(rootNode.children.head.token == "package")
  }

  test("Get the properties") {
    val properties = rootNode.children.head.properties
    assert(properties.size == 1)
    assert(properties("internalRole") == "package")
  }

  test("Simple XPath query called on client object andinstance") {
    var filtered = client.filter(rootNode, "//QualifiedName[@roleExpression]")
    var filtered2 = BblfshClient.filter(rootNode, "//QualifiedName[@roleExpression]")
    assert(filtered.length == 3)
    assert(filtered == filtered2)
  }

  test("Simple XPath query calling filter on the Node") {
    rootNode.filter("//QualifiedName[@roleExpression]")
  }

  test("XPath query with threads") {
    val th = new Thread(new Runnable {
      def run() {
        var filtered = rootNode.filter("//QualifiedName[@roleExpression]")
        assert(filtered.length == 3)
      }
    })
    th.start

    th.synchronized {
      th.wait
    }
  }

  test("XPath filter properties") {
    val filtered = rootNode.filter("//*[@internalRole='types']");
    assert(filtered.length == 1)
    val filteredNeg = rootNode.filter("//*[@internalRole='foo']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter StartOffset") {
    val filtered = rootNode.filter("//*[@startOffset='24']");
    assert(filtered.length == 1)
    val filteredNeg = rootNode.filter("//*[@startOffset='44']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter StartLine") {
    val filtered = rootNode.filter("//*[@startLine='1']");
    assert(filtered.length == 17)
    val filteredNeg = rootNode.filter("//*[@startLine='100']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter StartCol") {
    val filtered = rootNode.filter("//*[@startCol='25']");
    assert(filtered.length == 1)
    val filteredNeg = rootNode.filter("//*[@startCol='999']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter EndOffset") {
    val filtered = rootNode.filter("//*[@endOffset='44']");
    assert(filtered.length == 1)
    val filteredNeg = rootNode.filter("//*[@endOffset='999']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter EndLine") {
    val filtered = rootNode.filter("//*[@endLine='1']");
    assert(filtered.length == 16)
    val filteredNeg = rootNode.filter("//*[@endLine='100']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter EndCol") {
    val filtered = rootNode.filter("//*[@endCol='45']");
    assert(filtered.length == 1)
    val filteredNeg = rootNode.filter("//*[@endCol='999']");
    assert(filteredNeg.length == 0)
  }

  test("Get the start position") {
    val childWithPos = rootNode.children(1)
    val startPos = childWithPos.startPosition
    assert(!startPos.isEmpty)
    assert(startPos.get.offset == 24)
    assert(startPos.get.line == 1)
    assert(startPos.get.col == 25)
  }

  test("Get the end position") {
    val childWithPos = rootNode.children(1).children.head
    val endPos = childWithPos.endPosition
    assert(!endPos.isEmpty)
    assert(endPos.get.offset == 44)
    assert(endPos.get.line == 1)
    assert(endPos.get.col == 45)
  }
}

