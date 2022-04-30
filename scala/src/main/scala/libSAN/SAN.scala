package libSAN

object SAN {
    val ONES: Char = 0x3f

    val encTable: String = "+123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0-"
    val dec: String = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" +
        "\u0000" + // +
        "@" +
        "\u003f" + // -
        "@@" +
        "\u003e\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u0008\u0009" + // 0-9
        "@@@@@@@" +
        "\u0024\u0025\u0026\u0027\u0028\u0029\u002a\u002b\u002c\u002d\u002e\u002f\u0030" + // A-M
        "\u0031\u0032\u0033\u0034\u0035\u0036\u0037\u0038\u0039\u003a\u003b\u003c\u003d" + // N-Z
        "@@@@@@" +
        "\u000a\u000b\u000c\u000d\u000e\u000f\u0010\u0011\u0012\u0013\u0014\u0015\u0016" + // a-m
        "\u0017\u0018\u0019\u001a\u001b\u001c\u001d\u001e\u001f\u0020\u0021\u0022\u0023" + // n-z
        "@@@@@"

    private def enc(input: Long): Char = encTable((input & ONES).toInt)

    trait ValidationResult

    case object OK extends ValidationResult

    case object EMPTY extends ValidationResult

    case object HIGH_BIT extends ValidationResult

    case object WRONG_CHAR extends ValidationResult

    case object TOO_LONG extends ValidationResult

    def valid(input: String, bitSize: Int = 0): ValidationResult = {
        if (input.isEmpty) return EMPTY

        for (byte: Char <- input) {
            if (byte < 0 || byte >= 128) return HIGH_BIT
            if (dec(byte) >= 64) return WRONG_CHAR
        }

        if (bitSize > 0) {
            val maxSize: Int = (bitSize + 5) / 6
            val size: Int = input.length
            if (size > maxSize) return TOO_LONG
            if (size == maxSize) {
                val rest = bitSize % 6
                if (rest > 0) {
                    val firstByte = dec(input.head)
                    val usedBits = ((1 << rest) - 1).toChar
                    // signed
                    if ((firstByte & (1 << (rest - 1)).toChar) != 0) {
                        if ((firstByte | usedBits) != ONES) return TOO_LONG
                    }
                    // unsigned
                    else if ((firstByte & ~usedBits) != 0) return TOO_LONG
                }
            }
        }

        OK
    }

    def encode64(input: Long): String = {
        val blocks: Array[Char] = Array(
            enc(input >> 60 & ONES), enc(input >> 54 & ONES), enc(input >> 48 & ONES),
            enc(input >> 42 & ONES), enc(input >> 36 & ONES), enc(input >> 30 & ONES),
            enc(input >> 24 & ONES), enc(input >> 18 & ONES), enc(input >> 12 & ONES),
            enc(input >> 6 & ONES), enc(input & ONES)
        )

        for (i <- 0 to 9) {
            if (if (blocks(i + 1) != enc(ONES)) blocks(i) != enc(0) else blocks(i) != enc(ONES)) {
                return String.copyValueOf(blocks, i, blocks.length - i)
            }
        }

        blocks.last.toString
    }

    def decode64(input: String): Long = {
        var res = if (input.head == enc(ONES)) -1L else 0L
        for (byte <- input) {
            res = (res << 6) + dec(byte);
        }
        res
    }
}
