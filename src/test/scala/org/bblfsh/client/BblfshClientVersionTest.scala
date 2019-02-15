package org.bblfsh.client

import org.bblfsh.client.BblfshClient._

import gopkg.in.bblfsh.sdk.v1.protocol.generated.VersionResponse
import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import org.scalatest.FunSuite
import org.scalatest.BeforeAndAfter

import java.io.File
import scala.io.Source

class BblfshClientVersionTest extends FunSuite with BeforeAndAfter {

  val client = BblfshClient("0.0.0.0", 9432)
  val fileName = "src/test/resources/SampleJavaFile.java"
  val fileContent = Source.fromFile(fileName) .getLines.mkString
  var resp: VersionResponse = _

  before {
    resp = client.version()
  }

  test("Check version") {
    assert(!resp.version.isEmpty)
  }

  test("Check build") {
    assert(!resp.build.isEmpty)
  }

}

