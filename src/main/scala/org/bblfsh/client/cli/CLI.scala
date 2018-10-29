package org.bblfsh.client.cli

import java.io.File

import org.rogach.scallop._


class CLI(arguments: Seq[String]) extends ScallopConf(arguments) {
  val bblfshServerHost = opt[String](name = "host", noshort = true,
    default = Some("0.0.0.0"))
  val query = opt[String](name = "query", short = 'q', default = Some(""))
  val bblfshServerPort = opt[Int](name = "port", noshort = true, default = Some(9432),
    validate = (i => 0 < i && 65535 > i))
  val file = opt[File](name = "file", short = 'f', required = true)
  validateFileExists(file)
}
