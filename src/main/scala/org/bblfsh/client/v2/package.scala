package org.bblfsh.client

/** Bblfsh client for protocol v2.
  * See https://github.com/bblfsh/sdk/blob/v3.1.0/protocol/driver.proto
  *
  * The main class to use is [[org.bblfsh.client.v2.BblfshClient]]
  */
package object v2 {
  /** Key, Value representation of [[org.bblfsh.client.v2.JObject]] */
  type JField = (String, JNode)
  /** Aliases for constants coming from libuast
    * This is just for convenience, to be able to do
    * `import org.bblfsh.client.v2.TreeOrder`
    * instead of
    * `import org.bblfsh.client.v2.libuast.Libuast.TreeOrder`
    */
  type UastFormat = libuast.Libuast.UastFormat
  type TreeOrder = libuast.Libuast.TreeOrder
}
