package org.bblfsh.client.v2

import gopkg.in.bblfsh.sdk.v2.protocol.driver.VersionResponse
import org.scalatest.{BeforeAndAfter, FunSuite}

import scala.io.Source

class BblfshClientClose extends FunSuite with BeforeAndAfter {
  val client = BblfshClient("0.0.0.0", 9432)

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

