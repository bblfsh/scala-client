package org.bblfsh.client.v2

import gopkg.in.bblfsh.sdk.v2.protocol.driver.ParseResponse
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FlatSpec, Matchers}

import scala.io.Source

class BblfshClientBaseTest extends FlatSpec
  with BeforeAndAfter
  with BeforeAndAfterAll
  with Matchers {

  val client = BblfshClient("localhost", 9432)
  val fileName = "src/test/resources/SampleJavaFile.java"
  var resp: ParseResponse = _

  before {
    val fileContent = Source.fromFile(fileName).getLines.mkString("\n")
    resp = client.parse(fileName, fileContent)
  }

  override def afterAll {
    client.close()
    System.runFinalization()
    System.gc()
  }
}
