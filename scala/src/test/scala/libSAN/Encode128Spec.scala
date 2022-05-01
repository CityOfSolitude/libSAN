package libSAN

import libSAN.SAN.{OK, decode128, encode128, valid}
import org.scalatest.refspec.RefSpec

import scala.util.Random

class Encode128Spec extends RefSpec {

    def `important examples and corner cases work`(): Unit = {
        assert(encode128(0L, 0L) == "+")
        assert(decode128("+") == (0L, 0L))

        assert(encode128(0L, 1L) == "1")
        assert(decode128("1") == (0L, 1L))

        assert(encode128(-1L, -1L) == "-")
        assert(decode128("-") == (-1L, -1L))

        assert(encode128(-1L, -2L) == "-0")
        assert(decode128("-0") == (-1L, -2L))
    }

    def `exploring the full 22 bit space for both 64-bit parts shows no errors`(): Unit = {
        var counter = 0L
        for (i <- -1L << 21 until 1L << 21) {
            var encoded = encode128(0L, i)
            assert(valid(encoded, 128) == OK)
            assert(decode128(encoded) == (0, i))
            counter += 1

            encoded = encode128(i, i)
            assert(valid(encoded, 128) == OK)
            assert(decode128(encoded) == (i, i))
            counter += 1

            encoded = encode128(-1L, i)
            assert(valid(encoded, 128) == OK)
            assert(decode128(encoded) == (-1L, i))
            counter += 1

            encoded = encode128(i, 0L)
            assert(valid(encoded, 128) == OK)
            assert(decode128(encoded) == (i, 0))
            counter += 1
        }
        assert(counter == 1L << 24)
    }

    def `exploring random values from the full 128-bit space shows no errors`(): Unit = {
        val random = new Random()
        var counterPositive = 0L
        var counterNegative = 0L
        for (_ <- 0L until 1L << 17) {
            val ab = random.nextLong()
            val cd = random.nextLong()
            for (shift <- 0 until 128) {
                val (inputAB, inputCD) = if (shift <= 64) {
                    (ab >> shift, (ab << (64 - shift)) | (cd >> shift))
                } else {
                    (0L, ab >> (shift - 64))
                }
                if (ab >= 0) counterPositive += 1 else counterNegative += 1
                val encoded = encode128(inputAB, inputCD)
                assert(valid(encoded, 128) == OK)
                assert(decode128(encoded) == (inputAB, inputCD))
            }
        }
        assert(counterPositive + counterNegative == 1L << 24)
    }
}
