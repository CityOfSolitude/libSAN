package libSAN

import libSAN.SAN.{OK, decode48, decode48Signed, encode48, valid}
import org.scalatest.refspec.RefSpec

import scala.util.Random

class Encode48Spec extends RefSpec {

    def `important examples and corner cases work`(): Unit = {
        assert(encode48(0L) == "+")
        assert(decode48("+") == 0L)
        assert(decode48Signed("+") == 0L)

        assert(encode48(1L) == "1")
        assert(decode48("1") == 1L)
        assert(decode48Signed("1") == 1L)

        assert(encode48(-1L) == "-")
        assert(decode48("-") == 0xffffffffffffL)
        assert(decode48Signed("-") == -1L)

        assert(encode48(-2L) == "-0")
        assert(decode48("-0") == 0xfffffffffffeL)
        assert(decode48Signed("-0") == -2L)
    }

    def `exploring the full signed and unsigned 24 bit space shows no errors`(): Unit = {
        var counter = 0L
        for (i <- -1L << 23 until 1L << 24) {
            val encoded = encode48(i)
            assert(valid(encoded, 48) == OK)
            if (i < (1L << 23)) assert(decode48Signed(encoded) == i)
            if (i >= 0) assert(decode48(encoded) == i)
            counter += 1
        }
        assert(counter == (1L << 23) * 3) // 24 bit space and another "half" for negative (or high positives)
    }

    def `exploring random values from the full 48-bit space shows no errors`(): Unit = {
        val random = new Random()
        var counterPositive = 0L
        var counterNegative = 0L
        for (_ <- 0L until 1L << 19) {
            val base = random.nextLong() & 0xffffffffffffL
            for (shift <- 0 until 48) {
                val input = base >> shift
                if (input >= 0) counterPositive += 1 else counterNegative += 1
                val encoded = encode48(input)
                assert(valid(encoded, 48) == OK)
                val signed = decode48Signed(encoded)
                // since we did not input repeated sign bits, we must get rid of them in the output
                assert((signed & 0xffffffffffffL) == input)
                // still, the first two bytes should either be completely 0 or completely 1
                assert(signed >> 48 == 0L || signed >> 48 == -1L)
                // unsigned decoding just does what we expect
                assert(decode48(encoded) == input)
            }
        }
        // 2^19 times 48 shifts, which are 5-bit (32) "and a half"
        assert(counterPositive + counterNegative == (1L << 23) * 3)
    }
}
