package org.bblfsh.client

import gopkg.in.bblfsh.sdk.v1.protocol.generated.ParseResponse

import org.scalatest.FunSuite
import org.scalatest.BeforeAndAfter

import java.io.File
import scala.io.Source

class BblfshClientTest extends FunSuite with BeforeAndAfter {

  val client = BblfshClient("0.0.0.0", 9432)
  val fileName = "src/test/resources/SampleJavaFile.java"
  val fileContent = Source.fromFile(fileName) .getLines.mkString
  var resp: ParseResponse = _

  before {
    resp = client.parse(fileName, fileContent)
  }

  test("Parse UAST for existing .java file") {
    assert(resp.errors.isEmpty)
    assert(resp.uast.isDefined)
  }

  test("Get the internalType") {
    assert(resp.uast.get.internalType == "CompilationUnit")
  }

  test("Get the children") {
    val children = resp.uast.get.children
    assert(children.length == 2)
  }

  test("Get the token") {
    assert(resp.uast.get.children.head.token == "package")
  }

  test("Get the properties") {
    val properties = resp.uast.get.children.head.properties
    assert(properties.size == 1)
    assert(properties("internalRole") == "package")
  }

  test("Simple XPath query") {
    var filtered = client.filter(resp.uast.get, "//QualifiedName[@roleExpression]")
    assert(filtered.length == 3)
  }

  test("XPath query with threads") {
    val th = new Thread(new Runnable {
      def run() {
        var filtered = client.filter(resp.uast.get, "//QualifiedName[@roleExpression]")
        assert(filtered.length == 3)
      }
    })
    th.start

    th.synchronized {
      th.wait
    }
  }

  test("XPath filter properties") {
    val filtered = client.filter(resp.uast.get, "//*[@internalRole='types']");
    assert(filtered.length == 1)
    val filteredNeg = client.filter(resp.uast.get, "//*[@internalRole='foo']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter StartOffset") {
    val filtered = client.filter(resp.uast.get, "//*[@startOffset='24']");
    assert(filtered.length == 1)
    val filteredNeg = client.filter(resp.uast.get, "//*[@startOffset='44']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter StartLine") {
    val filtered = client.filter(resp.uast.get, "//*[@startLine='1']");
    assert(filtered.length == 17)
    val filteredNeg = client.filter(resp.uast.get, "//*[@startLine='100']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter StartCol") {
    val filtered = client.filter(resp.uast.get, "//*[@startCol='25']");
    assert(filtered.length == 1)
    val filteredNeg = client.filter(resp.uast.get, "//*[@startCol='999']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter EndOffset") {
    val filtered = client.filter(resp.uast.get, "//*[@endOffset='44']");
    assert(filtered.length == 1)
    val filteredNeg = client.filter(resp.uast.get, "//*[@endOffset='999']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter EndLine") {
    val filtered = client.filter(resp.uast.get, "//*[@endLine='1']");
    assert(filtered.length == 16)
    val filteredNeg = client.filter(resp.uast.get, "//*[@endLine='100']");
    assert(filteredNeg.length == 0)
  }

  test("Xpath filter EndCol") {
    val filtered = client.filter(resp.uast.get, "//*[@endCol='45']");
    assert(filtered.length == 1)
    val filteredNeg = client.filter(resp.uast.get, "//*[@endCol='999']");
    assert(filteredNeg.length == 0)
  }

  test("Get the start position") {
    val childWithPos = resp.uast.get.children(1)
    val startPos = childWithPos.startPosition
    assert(!startPos.isEmpty)
    assert(startPos.get.offset == 24)
    assert(startPos.get.line == 1)
    assert(startPos.get.col == 25)
  }

  test("Get the end position") {
    val childWithPos = resp.uast.get.children(1).children.head
    val endPos = childWithPos.endPosition
    assert(!endPos.isEmpty)
    assert(endPos.get.offset == 44)
    assert(endPos.get.line == 1)
    assert(endPos.get.col == 45)
  }

  test("Test non increasing memory usage with filter()") {
    System.gc()

    def someFilter() = {
      val node = resp.uast.get
      client.filter(node, "//QualifiedName[@roleExpression]")
      client.filter(node, "//*[@endLine='1']");
      client.filter(node, "//*[@internalRole='types']");
    }

    def getMemUsage = {
      System.gc()
      Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory()
    }
    someFilter
    val initialUsage = getMemUsage

    for (a<-1 to 10000) {
      someFilter
    }

    val finalUsage = getMemUsage
    assert(finalUsage <= initialUsage * 1.03)
  }
}

