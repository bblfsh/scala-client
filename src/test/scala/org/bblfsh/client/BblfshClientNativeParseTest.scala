package org.bblfsh.client

import org.bblfsh.client.BblfshClient._

import gopkg.in.bblfsh.sdk.v1.protocol.generated.NativeParseResponse
import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import org.scalatest.FunSuite
import org.scalatest.BeforeAndAfter

import java.io.File
import scala.io.Source

class BblfshClientNativeParseTest extends FunSuite with BeforeAndAfter {

  val client = BblfshClient("0.0.0.0", 9432)
  val fileName = "src/test/resources/SampleJavaFile.java"
  val fileContent = Source.fromFile(fileName).getLines.mkString("\n")
  var resp: NativeParseResponse = _
  var rootNode: String = _

  before {
    resp = client.nativeParse(fileName, fileContent)
    
    rootNode = resp.ast
  }

  test("Parse native AST for existing .java file") {
    assert(resp.errors.isEmpty)
    assert(!resp.ast.isEmpty)
  }

}

