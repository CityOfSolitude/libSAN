package libSAN

import libSAN.SAN._
import org.scalatest.refspec.RefSpec

class ValidSpec extends RefSpec {

    def `valid works for empty strings`(): Unit = {
        assert(valid("") == EMPTY)
        assert(valid("", 1) == EMPTY)
        assert(valid("", 10) == EMPTY)
        assert(valid("", 100) == EMPTY)
    }

    def `valid works for high bits and wrong chars`(): Unit = {
        var error_high = 0
        var error_wrong = 0
        var success = 0
        for (i <- -128 to 127) {
            for (j <- -128 to 127) {
                for (k <- -128 to 127) {
                    valid(Seq(i.toChar, j.toChar, k.toChar).mkString) match {
                        case HIGH_BIT => error_high += 1;
                        case WRONG_CHAR => error_wrong += 1;
                        case EMPTY => fail();
                        case _ => success += 1;
                    }
                }
            }
        }
        assert(success == 64 * 64 * 64)
        assert(error_high == 128 * 256 * 256 + 64 * 128 * 256 + 64 * 64 * 128)
        assert(error_wrong == 64 * 256 * 256 + 64 * 64 * 256 + 64 * 64 * 64)
    }

    def `valid works without size`(): Unit = {
        for (c <- encTable) {
            var encoded: String = ""
            for (_ <- 1 until 128) {
                encoded += c
                assert(OK.==(valid(encoded)))
            }
        }
    }

    def `valid works for too long first bytes with 6 bits`(): Unit = {
        for (c <- encTable) {
            assert(valid(Seq(c).mkString, 6) == OK)
            assert(valid(Seq(c, encTable(ONES)).mkString, 12) == OK)
        }
    }

    def `valid works for too long first bytes with 5 bits`(): Unit = {
        for (c <- encTable) {
            val sign: Boolean = (dec(c) & 0x10.toChar) != 0;
            val extra: Boolean = (dec(c) & 0x20.toChar) != 0;
            if (sign == extra) {
                assert(valid(Seq(c).mkString, 5) == OK);
                assert(valid(Seq(c, encTable(ONES)).mkString, 11) == OK);
                assert(valid(Seq(c, c).mkString, 11) == OK);
            } else {
                assert(valid(Seq(c).mkString, 5) == TOO_LONG);
                assert(valid(Seq(c, encTable(ONES)).mkString, 11) == TOO_LONG);
                assert(valid(Seq(c, c).mkString, 11) == TOO_LONG);
            }
        }
    }

    def `valid works for too long first bytes with 4 bits`(): Unit = {
        for (c <- encTable) {
            val sign: Boolean = (dec(c) & 0x08.toChar) != 0;
            val extra1: Boolean = (dec(c) & 0x20.toChar) != 0;
            val extra2: Boolean = (dec(c) & 0x10.toChar) != 0;
            if (sign == extra1 && sign == extra2) {
                assert(valid(Seq(c).mkString, 4) == OK);
                assert(valid(Seq(c, encTable(ONES)).mkString, 10) == OK);
                assert(valid(Seq(c, c).mkString, 10) == OK);
            } else {
                assert(valid(Seq(c).mkString, 4) == TOO_LONG);
                assert(valid(Seq(c, encTable(ONES)).mkString, 10) == TOO_LONG);
                assert(valid(Seq(c, c).mkString, 10) == TOO_LONG);
            }
        }
    }

    def `valid works for too long first bytes with 3 bits`(): Unit = {
        for (c <- encTable) {
            val sign: Boolean = (dec(c) & 0x04.toChar) != 0;
            val extra1: Boolean = (dec(c) & 0x20.toChar) != 0;
            val extra2: Boolean = (dec(c) & 0x10.toChar) != 0;
            val extra3: Boolean = (dec(c) & 0x08.toChar) != 0;
            if (sign == extra1 && sign == extra2 && sign == extra3) {
                assert(valid(Seq(c).mkString, 3) == OK);
                assert(valid(Seq(c, encTable(ONES)).mkString, 9) == OK);
                assert(valid(Seq(c, c).mkString, 9) == OK);
            } else {
                assert(valid(Seq(c).mkString, 3) == TOO_LONG);
                assert(valid(Seq(c, encTable(ONES)).mkString, 9) == TOO_LONG);
                assert(valid(Seq(c, c).mkString, 9) == TOO_LONG);
            }
        }
    }

    def `valid works for too long first bytes with 2 bits`(): Unit = {
        for (c <- encTable) {
            val sign: Boolean = (dec(c) & 0x02.toChar) != 0;
            val extra1: Boolean = (dec(c) & 0x20.toChar) != 0;
            val extra2: Boolean = (dec(c) & 0x10.toChar) != 0;
            val extra3: Boolean = (dec(c) & 0x08.toChar) != 0;
            val extra4: Boolean = (dec(c) & 0x04.toChar) != 0;
            if (sign == extra1 && sign == extra2 && sign == extra3 && sign == extra4) {
                assert(valid(Seq(c).mkString, 2) == OK);
                assert(valid(Seq(c, encTable(ONES)).mkString, 8) == OK);
                assert(valid(Seq(c, c).mkString, 8) == OK);
            } else {
                assert(valid(Seq(c).mkString, 2) == TOO_LONG);
                assert(valid(Seq(c, encTable(ONES)).mkString, 8) == TOO_LONG);
                assert(valid(Seq(c, c).mkString, 8) == TOO_LONG);
            }
        }
    }

    def `valid works for too long first bytes with 1 bit`(): Unit = {
        for (c <- encTable) {
            val sign: Boolean = (dec(c) & 0x01.toChar) != 0;
            val extra1: Boolean = (dec(c) & 0x20.toChar) != 0;
            val extra2: Boolean = (dec(c) & 0x10.toChar) != 0;
            val extra3: Boolean = (dec(c) & 0x08.toChar) != 0;
            val extra4: Boolean = (dec(c) & 0x04.toChar) != 0;
            val extra5: Boolean = (dec(c) & 0x02.toChar) != 0;
            if (sign == extra1 && sign == extra2 && sign == extra3 && sign == extra4 && sign == extra5) {
                assert(valid(Seq(c).mkString, 1) == OK);
                assert(valid(Seq(c, encTable(ONES)).mkString, 7) == OK);
                assert(valid(Seq(c, c).mkString, 7) == OK);
            } else {
                assert(valid(Seq(c).mkString, 1) == TOO_LONG);
                assert(valid(Seq(c, encTable(ONES)).mkString, 7) == TOO_LONG);
                assert(valid(Seq(c, c).mkString, 7) == TOO_LONG);
            }
        }
    }
}
