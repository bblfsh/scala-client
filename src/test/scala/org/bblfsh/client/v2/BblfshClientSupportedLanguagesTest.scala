package org.bblfsh.client.v2

import gopkg.in.bblfsh.sdk.v2.protocol.driver.SupportedLanguagesResponse
import org.scalatest.{BeforeAndAfter, FunSuite}

class BblfshClientSupportedLanguagesTest extends FunSuite with BeforeAndAfter {
  val client = BblfshClient("0.0.0.0", 9432)
  var resp: SupportedLanguagesResponse = _

  before {
    resp = client.supportedLanguages()
  }

  test("Check languages are not empty") {
    assert(!resp.languages.isEmpty)
  }

  test("Check languages contain Java") {
    assert(resp.languages.map(d => d.name).contains("Java"))
  }
}
