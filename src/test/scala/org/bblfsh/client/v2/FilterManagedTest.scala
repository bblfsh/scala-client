package org.bblfsh.client.v2

import gopkg.in.bblfsh.sdk.v2.protocol.driver.Mode
import org.bblfsh.client.v2.libuast.Libuast
import org.scalatest.prop.TableDrivenPropertyChecks._

import scala.io.Source

class FilterManagedTest extends BblfshClientBaseTest {

  import BblfshClient._ // enables uast.* methods

  val ctx: Context = Context()
  val managedRoot = JArray(
    JObject(
      "@type" -> JString("file"),
      "k1" -> JString("v1"),
      "k2" -> JObject(
        "k3" -> JInt(24),
        "k4" -> JFloat(1.0),
        "k5" -> JArray(
          JObject(
            "k6" -> JBool(true),
            "k7" -> JString("v2"),
            "k8" -> JObject(
              "k9" -> JBool(false),
              "k10" -> JFloat(2.0)
            )
          )
        )
      )
    )
  )

  "XPath filter" should "find all positions under context" in {
    val it = ctx.filter("//file", managedRoot)
    it.hasNext() should be(true)

    val pos = it.toList
    pos should have size (1) // managedRoot contains only a node JObject

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

  val valueFilters =
    Table(
      ("filter", "expected"),
      (BblfshClient.filterInt _, List(JInt(24))),
      (BblfshClient.filterBool _, List(JBool(true), JBool(false))),
      (BblfshClient.filterString _, List(JString("file"), JString("v1"), JString("v2"))),
      (BblfshClient.filterFloat _, List(JFloat(1.0), JFloat(2.0)))
    )

  "Filtering UAST" should "work for Value types" in {
    val iterCount = BblfshClient.filterInt(managedRoot, "count(//*)")
    val allNodesIt = BblfshClient.filter(managedRoot, "//*")
    val numNodes = allNodesIt.size
    iterCount.toList should be (List(JInt(numNodes))) // number of nodes

    forAll (valueFilters) { (filter, expected: List[JNode]) =>
      val it = filter(managedRoot, "//*")
      it.toList should be (expected)
    }
  }

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
