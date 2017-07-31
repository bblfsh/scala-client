package org.bblfsh.client

import java.io.File

import org.scalatest.{BeforeAndAfterAll, FunSuite}

import scala.io.Source


class BblfshClientTest extends FunSuite with BeforeAndAfterAll {

  override def beforeAll() = {

  }

  test("Parse UAST for existing .py file") {
    // given
    // Bblfsh server is running on port :9432
    val client = BblfshClient("0.0.0.0", 9432)
    val filename = "src/test/resources/python_file.py" // client read it, and encode to utf-8
    assert(new File(filename).exists())
    val fileContent = Source.fromFile(filename) .getLines.mkString

    // when
    val resp = client.parse(filename, fileContent)

    //then
    assert(resp.errors.isEmpty)
    assert(resp.uast.isDefined)
  }

}
