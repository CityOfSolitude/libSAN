package libSAN

import libSAN.SAN.{OK, decode24, decode24Signed, encode24, valid}
import org.scalatest.refspec.RefSpec

import scala.util.Random

class Encode24Spec extends RefSpec {

    def `important examples and corner cases work`(): Unit = {
        assert(encode24(0) == "+")
        assert(decode24("+") == 0)
        assert(decode24Signed("+") == 0)

        assert(encode24(1) == "1")
        assert(decode24("1") == 1)
        assert(decode24Signed("1") == 1)

        assert(encode24(-1) == "-")
        assert(decode24("-") == 0xffffff)
        assert(decode24Signed("-") == -1)

        assert(encode24(-2) == "-0")
        assert(decode24("-0") == 0xfffffe)
        assert(decode24Signed("-0") == -2)
    }

    def `exploring the full signed and unsigned 24 bit space shows no errors`(): Unit = {
        var counter = 0
        for (i <- -1 << 23 until 1 << 24) {
            val encoded = encode24(i)
            assert(valid(encoded, 24) == OK)
            if (i < (1 << 23)) assert(decode24Signed(encoded) == i)
            if (i >= 0) assert(decode24(encoded) == i)
            counter += 1
        }
        assert(counter == (1 << 23) * 3) // 24 bit space and another "half" for negative (or high positives)
    }

    def `exploring random values from the full 24-bit space shows no errors`(): Unit = {
        val random = new Random()
        var counterPositive = 0
        var counterNegative = 0
        for (_ <- 0 until 1 << 20) {
            val base = random.nextInt() & 0xffffff
            for (shift <- 0 until 24) {
                val input = base >> shift
                if (input >= 0) counterPositive += 1 else counterNegative += 1
                val encoded = encode24(input)
                assert(valid(encoded, 24) == OK)
                val signed = decode24Signed(encoded)
                // since we did not input repeated sign bits, we must get rid of them in the output
                assert((signed & 0xffffff) == input)
                // still, the first two bytes should either be completely 0 or completely 1
                assert(signed >> 24 == 0 || signed >> 24 == -1)
                // unsigned decoding just does what we expect
                assert(decode24(encoded) == input)
            }
        }
        // 2^20 times 24 shifts, which are 4-bit (16) "and a half"
        assert(counterPositive + counterNegative == (1 << 23) * 3)
    }
}
