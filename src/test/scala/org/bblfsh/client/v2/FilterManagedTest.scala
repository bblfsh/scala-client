package org.bblfsh.client.v2

import gopkg.in.bblfsh.sdk.v2.protocol.driver.Mode
import org.bblfsh.client.v2.libuast.Libuast

import scala.io.Source

class FilterManagedTest extends BblfshClientBaseTest {

  import BblfshClient._ // enables uast.* methods

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
    super.beforeAll()
    System.err.println(s"Libuast.loaded: ${Libuast.loaded}")
    // to load native JNI lib \wo the full client
  }

  override def beforeEach() = {
    super.beforeEach()
    ctx = Context()
  }

  override def afterAll() = {
    super.afterAll()
    System.runFinalization()
    System.gc()
  }

  "XPath filter" should "find all positions under context" in {
    val it = ctx.filter("//file", managedRoot)
    it.hasNext() should be(true)

    val pos = it.toList
    pos should have size (1) // Tiny.java contains 1 file node

    it.close()
    it.hasNext() should be(false)
  }

  "Filtering UAST" should "work in Native mode" in {
    val fileContent = Source.fromFile(fileName).getLines.mkString("\n")
    val resp = client.parse(fileName, fileContent, Mode.NATIVE)
    val node = resp.get

    val iter = BblfshClient.filter(node, "//SimpleName")
    iter.toList should have size (10) // number of Identifiers in the file
    iter.close()
  }

  // TODO(#110) implement value type returns
  //  "Filtering UAST" should "work for Value types" in {
  //    val iter = BblfshClient.filterNumber(resp.get, "count(//*)")
  //    iter.toList should have size (517) // total number of nodes (not the number of results which is 1)
  //  }

  "Filtering UAST" should "work in Annotated mode" in {
    val fileContent = Source.fromFile(fileName).getLines.mkString("\n")
    val resp = client.parse(fileName, fileContent, Mode.ANNOTATED)
    val node = resp.get

    val iter = BblfshClient.filter(node, "//SimpleName[@role='Call']")
    iter.toList should have size (1) // number of function called in the file
    iter.close()
  }

  "Filtering UAST" should "work in Semantic mode" in {
    val fileContent = Source.fromFile(fileName).getLines.mkString("\n")
    val resp = client.parse(fileName, fileContent, Mode.SEMANTIC)
    val node = resp.get

    val iter = BblfshClient.filter(node, "//uast:Identifier[@role='Call']")
    iter.toList should have size (1) // number of function called in the file
    iter.close()
  }

}
