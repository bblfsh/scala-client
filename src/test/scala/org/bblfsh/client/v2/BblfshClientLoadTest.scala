package org.bblfsh.client.v2


class BblfshClientLoadTest extends BblfshClientBaseTest {

  import BblfshClient._ // enables uast.* methods

  override val fileName = "src/test/resources/Tiny.java"

  "Loading Go -> JVM of a real tree" should "bring JNode tree to memory" in {
    val uast = resp.uast.decode()
    val rootNode: Node = uast.root()

    println(s"Loading $rootNode")
    val root = rootNode.load()

    root should not be Nil
    root.getClass shouldBe classOf[JObject]
    root.children should not be empty
    root.children.size shouldBe 6

    val arr = root.children(1)
    arr should not be (null)
    arr.getClass shouldBe classOf[JArray]
    arr.children.size shouldBe 1

    val str = arr.children(0)
    str.getClass shouldBe classOf[JString]

    val nil = root("imports")
    nil should be (JNull())

    println(s"Result size: ${root.children.size}\n")
    root.children.foreach(println)
  }

  // TODO(#90) a stub for testing JNode encoding impl
  /*"Loading Go -> JVM for a simple encoded tree" should "bring JNode tree to memory" in {
    val rootTree: JNode = JArray(Buffer(
      JObject(Buffer(
        "k1" -> JString("v1")
      )),
      JString("test")
    ))

    val ctx = Context()

    ctx.encode(rootTree)
  }*/

  // TODO(#90) a stub for more extensive testing of JNode encoding impl
  /*"Decoding, loading & encoding to different context" should "produce the same results" in {
    val uast = resp.uast.decode()
    val rootNode: Node = uast.root()

    println(s"Loading $rootNode")
    val root = rootNode.load()

    val ctx = Context()
    val data = ctx.encode(root)

    data shouldBe equal resp.uast
  }*/

}
