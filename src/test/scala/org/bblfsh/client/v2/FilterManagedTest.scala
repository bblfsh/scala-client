package org.bblfsh.client.v2

import org.bblfsh.client.v2.libuast.Libuast
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FlatSpec, Matchers}

class FilterManagedTest extends FlatSpec
  with Matchers
  with BeforeAndAfter
  with BeforeAndAfterAll {

  var ctx: Context = _
  val managedRoot = JArray(
    JObject(
      "@type" -> JString("file"),
      "k1" -> JString("v1"),
      "k2" -> JObject(
        "k3" -> JInt(24)
      )
    ))

  override def beforeAll() = {
    println(s"Libuast.loaded: ${Libuast.loaded}")
    // to load native JNI lib \wo the full client
  }

  before {
    ctx = Context()
  }

  "XPath filter" should "find all positions under context" in {
    val it = ctx.filter("//file", managedRoot)
    it.hasNext() should be(true)

    val pos = it.toList
    pos should have size (1)

    it.close()
    it.hasNext() should be(false)
  }

}
