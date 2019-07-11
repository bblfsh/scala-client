package org.bblfsh.client.v2

import org.scalatest.{BeforeAndAfter, FlatSpec, Matchers}

import scala.io.Source

class ContextTest extends FlatSpec
  with BeforeAndAfter
  with Matchers {


  "ContextExt" should "be able to .dispose() twice" in {
    val client = BblfshClient("localhost", 9432)
    val fileName = "src/test/resources/SampleJavaFile.java"
    val fileContent = Source.fromFile(fileName).getLines.mkString("\n")
    val resp = client.parse(fileName, fileContent)

    import BblfshClient._ // enables uast.* methods

    val ctx = resp.uast.decode()
    ctx.dispose()
    ctx.dispose()
  }


}
