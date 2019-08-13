package org.bblfsh.client.v2

import org.scalatest.{BeforeAndAfter, FlatSpec, Matchers}

class FilterManagedTest extends FlatSpec
  with Matchers
  with BeforeAndAfter {

  var ctx: Context = _
  val managedRoot = JArray(
    JObject(
      "@type" -> JString("file"),
      "k1" -> JString("v1"),
      "k2" -> JObject(
        "k3" -> JInt(24)
      )
    ))

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
