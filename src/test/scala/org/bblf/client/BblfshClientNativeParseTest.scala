package org.bblfsh.client

import gopkg.in.bblfsh.sdk.v2.protocol.driver.{Mode, ParseResponse}
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FunSuite}

import scala.io.Source

class BblfshClientNativeParseTest extends FunSuite
  with BeforeAndAfter
  with BeforeAndAfterAll {

  val client = BblfshClient("127.0.0.1", 9432)
  val fileName = "src/test/resources/SampleJavaFile.java"
  val fileContent = Source.fromFile(fileName).getLines.mkString("\n")
  var resp: ParseResponse = _

  override def afterAll {
    client.close()
  }

  test("Parse native AST for existing .java file") {
    resp = client.parse(fileName, fileContent, mode = Mode.NATIVE)

    assert(resp.errors.isEmpty)
    assert(!resp.uast.isEmpty)
  }

}

