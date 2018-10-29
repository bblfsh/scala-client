package org.bblf.client

//import gopkg.in.bblfsh.sdk.v1.protocol.generated.SupportedLanguagesResponse
import org.bblfsh.client.BblfshClient
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FunSuite}

//TODO(bzz) download sdk.v1 as well and generate code for SupportedLanguages() and Version()
class BblfshClientSupportedLanguagesTest extends FunSuite
  with BeforeAndAfter
  with BeforeAndAfterAll {
/*
  val client = BblfshClient("0.0.0.0", 9432)
  var resp: SupportedLanguagesResponse = _

  before {
    resp = client.supportedLanguages()
  }

  override def afterAll {
    client.close()
  }

  test("Check languages are not empty") {
    assert(!resp.languages.isEmpty)
  }

  test("Check languages contain Java") {
    assert(resp.languages.map(d => d.name).contains("Java"))
  }
  */
}
