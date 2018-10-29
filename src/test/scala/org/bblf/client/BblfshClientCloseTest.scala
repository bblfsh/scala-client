package org.bblfsh.client

import org.scalatest.{BeforeAndAfter, FunSuite}

class BblfshClientCloseTest extends FunSuite with BeforeAndAfter {

  val client = BblfshClient("127.0.0.1", 9432)

  test("Check close") {
    // call client method to check connection works
    val resp = client.parse("test.go", "package main")
    assert(!resp.language.isEmpty)

    client.close()

    // now client method should throw exception:
    // io.grpc.StatusRuntimeException: UNAVAILABLE: Channel shutdown invoked
    assertThrows[io.grpc.StatusRuntimeException] {
      client.parse("test.go", "")
    }
  }
}

