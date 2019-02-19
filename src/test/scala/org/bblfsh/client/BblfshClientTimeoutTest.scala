package org.bblfsh.client

import org.scalatest.{BeforeAndAfter, FunSuite}

import scala.io.Source

class BblfshClientTimeoutTest extends FunSuite with BeforeAndAfter {

  val client = BblfshClient("0.0.0.0", 9432)
  val fileName = "src/test/resources/large.php"
  val fileContent = Source.fromFile(fileName).getLines.mkString("\n")

  test("Parse requests with default timeout fails") {
    val resp = client.parse(fileName, fileContent)
    assert(!resp.errors.isEmpty)
    assert(resp.errors(0).startsWith("rpc error: code = DeadlineExceeded desc"))
  }

  test("Parse requests with 1 min timeout works") {
    val resp = client.parseWithTimeout(fileName, fileContent, 60)
    assert(resp.errors.isEmpty)
  }
}
