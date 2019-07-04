package org.bblfsh.client.v2

import org.scalatest.{BeforeAndAfter, FunSuite}


class BblfshClientClose extends FunSuite with BeforeAndAfter {
  val client = BblfshClient("localhost", 9432)

  test("Check close") {
    // call client method to check connection works
    val resp = client.version()
    assert(!resp.version.isEmpty)

    client.close()

    // now client method should throw exception:
    // io.grpc.StatusRuntimeException: UNAVAILABLE: Channel shutdown invoked
    assertThrows[io.grpc.StatusRuntimeException] {
      client.version()
    }
  }
}

