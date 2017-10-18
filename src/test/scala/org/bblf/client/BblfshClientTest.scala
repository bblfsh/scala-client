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

   //XXX uncomment
  //test("Simple XPath query") {
    //var filtered = client.filter(resp.uast.get, "//QualifiedName[@roleExpression]")
    //assert(filtered.length == 3)
  //}

  //test("XPath query with threads") {
    //val th = new Thread(new Runnable {
      //def run() {
        //var filtered = client.filter(resp.uast.get, "//QualifiedName[@roleExpression]")
        //assert(filtered.length == 3)
      //}
    //})
    //th.start

    //th.synchronized {
      //th.wait
    //}
  //}

  //test("XPath filter properties") {
    //val filtered = client.filter(resp.uast.get, "//*[@internalRole='types']");
    //assert(filtered.length == 1)
    //val filteredNeg = client.filter(resp.uast.get, "//*[@internalRole='foo']");
    //assert(filteredNeg.length == 0)
  //}

  //test("Xpath filter StartOffset") {
    //val filtered = client.filter(resp.uast.get, "//*[@startOffset='24']");
    //assert(filtered.length == 1)
    //val filteredNeg = client.filter(resp.uast.get, "//*[@startOffset='44']");
    //assert(filteredNeg.length == 0)
  //}

  //test("Xpath filter StartLine") {
    //val filtered = client.filter(resp.uast.get, "//*[@startLine='1']");
    //assert(filtered.length == 17)
    //val filteredNeg = client.filter(resp.uast.get, "//*[@startLine='100']");
    //assert(filteredNeg.length == 0)
  //}

  //test("Xpath filter StartCol") {
    //val filtered = client.filter(resp.uast.get, "//*[@startCol='25']");
    //assert(filtered.length == 1)
    //val filteredNeg = client.filter(resp.uast.get, "//*[@startCol='999']");
    //assert(filteredNeg.length == 0)
  //}

  //test("Xpath filter EndOffset") {
    //val filtered = client.filter(resp.uast.get, "//*[@endOffset='44']");
    //assert(filtered.length == 1)
    //val filteredNeg = client.filter(resp.uast.get, "//*[@endOffset='999']");
    //assert(filteredNeg.length == 0)
  //}

  //test("Xpath filter EndLine") {
    //val filtered = client.filter(resp.uast.get, "//*[@endLine='1']");
    //assert(filtered.length == 16)
    //val filteredNeg = client.filter(resp.uast.get, "//*[@endLine='100']");
    //assert(filteredNeg.length == 0)
  //}

  //test("Xpath filter EndCol") {
    //val filtered = client.filter(resp.uast.get, "//*[@endCol='45']");
    //assert(filtered.length == 1)
    //val filteredNeg = client.filter(resp.uast.get, "//*[@endCol='999']");
    //assert(filteredNeg.length == 0)
  //}

  test("Get the start position") {
    val childWithPos = resp.uast.get.children(1)
    val startPos = childWithPos.startPosition
    assert(!startPos.isEmpty)
    assert(startPos.get.offset == 24)
    assert(startPos.get.line == 1)
    assert(startPos.get.col == 25)
  }

  test("Internal Has[Start|End]Position") {
    val childWithPos = resp.uast.get.children(1)
    val startPos = childWithPos.startPosition

    // XXX remove
    assert(client.hasstartposition(childWithPos))
    assert(!client.hasstartposition(resp.uast.get))
    assert(client.hasendposition(childWithPos))
    assert(!client.hasendposition(resp.uast.get))
  }

  test("Get the end position") {
    // XXX report a issue for the java driver for the endposition of the 1st node
    val childWithPos = resp.uast.get.children(1).children.head
    val endPos = childWithPos.endPosition
    assert(!endPos.isEmpty)
    assert(endPos.get.offset == 44)
    assert(endPos.get.line == 1)
    assert(endPos.get.col == 45)
  }

  // XXX remove
  test("Internal PropertyKeyValueAt") {
    val node = resp.uast.get.children(1)
    val properties = node.properties
    println("XXX foo:")
    println(client.propertykeyat(node, 0))
    assert(client.propertykeyat(node, 0) == "interface")
    assert(client.propertyvalueat(node, 0) == "false")
    assert(client.propertykeyat(node, 1) == "internalRole")
    assert(client.propertyvalueat(node, 1) == "types")
  }

  test("[Start|End][Offset|Line|Col]") {
    val childWithPos = resp.uast.get.children(1).children.head
    assert(client.startoffset(childWithPos) == 30)
    assert(client.startline(childWithPos) == 1)
    assert(client.startcol(childWithPos) == 31)

    assert(client.endoffset(childWithPos) == 44)
    assert(client.endline(childWithPos) == 1)
    assert(client.endcol(childWithPos) == 45)
  }
}

