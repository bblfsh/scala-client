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
}
