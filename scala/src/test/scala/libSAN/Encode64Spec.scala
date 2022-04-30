package libSAN

import libSAN.SAN.{OK, decode64, encode64, valid}
import org.scalatest.refspec.RefSpec

import scala.util.Random

class Encode64Spec extends RefSpec {

    def `important examples and corner cases work`(): Unit = {
        assert(encode64(0L) == "+")
        assert(decode64("+") == 0L)

        assert(encode64(1L) == "1")
        assert(decode64("1") == 1L)

        assert(encode64(-1L) == "-")
        assert(decode64("-") == -1L)

        assert(encode64(-2L) == "-0")
        assert(decode64("-0") == -2L)
    }

    def `exploring the full 24 bit space shows no errors`(): Unit = {
        var counter = 0L
        for (i <- -1L << 23 until 1L << 23) {
            val encoded = encode64(i)
            assert(valid(encoded, 64) == OK)
            assert(decode64(encoded) == i)
            counter += 1
        }
        assert(counter == 1L << 24)
    }

    def `exploring random values from the full 64-bit space shows no errors`(): Unit = {
        val random = new Random()
        var counterPositive = 0L
        var counterNegative = 0L
        for (_ <- 0L until 1L << 18) {
            val base = random.nextLong()
            for (shift <- 0 until 64) {
                val input = base >> shift
                if (input >= 0) counterPositive += 1 else counterNegative += 1
                val encoded = encode64(input)
                assert(valid(encoded, 64) == OK)
                assert(decode64(encoded) == input)
            }
        }
        assert(counterPositive + counterNegative == 1L << 24)
    }
}
