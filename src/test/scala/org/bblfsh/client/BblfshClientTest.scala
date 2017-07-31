package org.bblfsh.client

import java.io.File

import org.scalatest.FunSuite

import scala.io.Source


class BblfshClientTest extends FunSuite {

  //val filename = "src/test/resources/python_file.py" // driver fails with "Exception: Could not determine Python version"

  test("Parse UAST for existing .java file") {
    // given
    // Bblfsh server is running on port :9432
    val client = BblfshClient("0.0.0.0", 9432)
    val filename = "src/test/resources/SampleJavaFile.java" // client read it, and encode to utf-8
    assert(new File(filename).exists())
    val fileContent = Source.fromFile(filename) .getLines.mkString

    // when
    val resp = client.parse(filename, fileContent)

    //then
    assert(resp.errors.isEmpty)
    assert(resp.uast.isDefined)
  }

}
