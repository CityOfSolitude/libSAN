package libSAN

import libSAN.SAN.{OK, decode32, encode32, valid}
import org.scalatest.refspec.RefSpec

import scala.util.Random

class Encode32Spec extends RefSpec {

    def `important examples and corner cases work`(): Unit = {
        assert(encode32(0) == "+")
        assert(decode32("+") == 0)

        assert(encode32(1) == "1")
        assert(decode32("1") == 1)

        assert(encode32(-1) == "-")
        assert(decode32("-") == -1)

        assert(encode32(-2) == "-0")
        assert(decode32("-0") == -2)
    }

    def `exploring the full 24 bit space shows no errors`(): Unit = {
        var counter = 0
        for (i <- -1 << 23 until 1 << 23) {
            val encoded = encode32(i)
            assert(valid(encoded, 32) == OK)
            assert(decode32(encoded) == i)
            counter += 1
        }
        assert(counter == 1 << 24)
    }

    def `exploring random values from the full 32-bit space shows no errors`(): Unit = {
        val random = new Random()
        var counterPositive = 0
        var counterNegative = 0
        for (_ <- 0 until 1 << 19) {
            val base = random.nextInt()
            for (shift <- 0 until 32) {
                val input = base >> shift
                if (input >= 0) counterPositive += 1 else counterNegative += 1
                val encoded = encode32(input)
                assert(valid(encoded, 32) == OK)
                assert(decode32(encoded) == input)
            }
        }
        assert(counterPositive + counterNegative == 1 << 24)
    }
}
