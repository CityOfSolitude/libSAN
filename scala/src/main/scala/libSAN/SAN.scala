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

    sealed trait ValidationResult

    case object OK extends ValidationResult

    case object EMPTY extends ValidationResult

    case object HIGH_BIT extends ValidationResult

    case object WRONG_CHAR extends ValidationResult

    case object TOO_LONG extends ValidationResult

    /**
     * Determines whether the string is a valid encoding, i.e., all characters
     * are from the encoding table. It does NOT consider the length of the
     * string, besides it being empty.
     *
     * If a bit size is given, it also tests total length of the string and
     * whether the first character of a full string is from the valid subset
     * of the encoding table.
     *
     * @param input   a string that might be the result of a prior encoding
     * @param bitSize (optional) length of the originally encoded input
     * @return whether any errors would occur while decoding
     */
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

    /**
     * Helper function to share the common logic of sparseness
     *
     * @param blocks all encoding blocks, in order
     * @return the sparse string that uniquely identifies this sequence of blocks
     */
    private def encodeBlocks(blocks: Array[Char]): String = {
        for (i <- blocks.indices.init) {
            if (if (blocks(i + 1) != enc(ONES)) blocks(i) != enc(0) else blocks(i) != enc(ONES)) {
                return String.copyValueOf(blocks, i, blocks.length - i)
            }
        }
        blocks.last.toString
    }


    /**
     * Encodes a 3-byte input value into an up-to 4-byte output string.
     * The first byte is irrelevant and will be ignored.
     * Signedness will just work, within the bound of a signed 24 bit value.
     *
     * The output will be as short as possible, by omitting leading 0 blocks.
     * Also we omit repeated, leading blocks of 1s, with the downside of having
     * to explicitly mark some positive numbers with a leading 1s block with an
     * additional 0s block.
     *
     * @param input a 24 bit value, encoded within a 32 bit value
     * @return a non-empty encoding of the input value.
     */
    def encode24(input: Int): String = encodeBlocks(Array(
        enc(input >> 18), enc(input >> 12), enc(input >> 6), enc(input)
    ))

    /**
     * Encodes a 4-byte input value into an up-to 6-byte output string.
     *
     * The output will be as short as possible, by omitting leading 0 blocks.
     * Also we omit repeated, leading blocks of 1s, with the downside of having
     * to explicitly mark some positive numbers with a leading 1s block with an
     * additional 0s block.
     *
     * For 32-bit values, we are left with a 2-bit high block, that will only ever
     * be encoded with the 000000, 000001, 111110 and 111111 block (since we honor
     * the sign!). For our encoding this means '+', '1', '0' or '-', respectively.
     *
     * @param input a 32 bit value
     * @return a non-empty encoding of the input value.
     */
    def encode32(input: Int): String = encodeBlocks(Array(
        enc(input >> 30), enc(input >> 24), enc(input >> 18), enc(input >> 12), enc(input >> 6), enc(input)
    ))

    /**
     * Encodes a 6-byte input value into an up-to 8-byte output string.
     * The first two bytes are irrelevant and will be ignored.
     * Signedness will just work, within the bound of a signed 48 bit value.
     *
     * The output will be as short as possible, by omitting leading 0 blocks.
     * Also we omit repeated, leading blocks of 1s, with the downside of having
     * to explicitly mark some positive numbers with a leading 1s block with an
     * additional 0s block.
     *
     * @param input a 24 bit value, encoded within a 64 bit value
     * @return a non-empty encoding of the input value.
     */
    def encode48(input: Long): String = encodeBlocks(Array(
        enc(input >> 42), enc(input >> 36), enc(input >> 30), enc(input >> 24),
        enc(input >> 18), enc(input >> 12), enc(input >> 6), enc(input)
    ))

    /**
     * Encodes a 8-byte input value into an up-to 11-byte output string.
     *
     * The output will be as short as possible, by omitting leading 0 blocks.
     * Also we omit repeated, leading blocks of 1s, with the downside of having
     * to explicitly mark some positive numbers with a leading 1s block with an
     * additional 0s block.
     *
     * For 64-bit values, we are left with a 4-bit high block, that will only ever
     * be encoded with the 16 different blocks (since we honor the sign!).
     *
     * @param input a 64 bit value
     * @return a non-empty encoding of the input value.
     */
    def encode64(input: Long): String = encodeBlocks(Array(
        enc(input >> 60), enc(input >> 54), enc(input >> 48), enc(input >> 42), enc(input >> 36),
        enc(input >> 30), enc(input >> 24), enc(input >> 18), enc(input >> 12), enc(input >> 6), enc(input)
    ))

    /**
     * Encodes a 16-byte input value into an up-to 22-byte output string.
     *
     * The output will be as short as possible, by omitting leading 0 blocks.
     * Also we omit repeated, leading blocks of 1s, with the downside of having
     * to explicitly mark some positive numbers with a leading 1s block with an
     * additional 0s block.
     *
     * @param ab the first 64 bit value
     * @param cd the first 64 bit value
     * @return a non-empty encoding of the input value.
     */
    def encode128(ab: Long, cd: Long): String = encodeBlocks(Array(
        enc(ab >> 62), enc(ab >> 56), enc(ab >> 50), enc(ab >> 44), enc(ab >> 38), enc(ab >> 32),
        enc(ab >> 26), enc(ab >> 20), enc(ab >> 14), enc(ab >> 8), enc(ab >> 2),
        enc((ab << 4 & 0x30) | (cd >> 60 & 0x0f)), enc(cd >> 54), enc(cd >> 48), enc(cd >> 42),
        enc(cd >> 36), enc(cd >> 30), enc(cd >> 24), enc(cd >> 18), enc(cd >> 12), enc(cd >> 6), enc(cd)
    ))

    /**
     * Decodes a previously encoded 3-byte value from its string representation.
     *
     * @param input a 1-4 byte string, which was the output of a previous encoding call
     * @return the decoded 24 bit value, interpreted as unsigned value
     */
    def decode24(input: String): Long = decode32(input) & ((1 << 24) - 1)

    /**
     * Convenience method for signed values; the decoding does not differ from
     * the unsigned one, with the exception of the first 8 bits, which will repeat
     * the sign of the 24 bit number.
     *
     * @param input a 1-4 byte string, which was the output of a previous encoding call
     * @return the decoded 24 bit value, with the highest 8 bit replicating the sign
     */
    def decode24Signed(input: String): Long = {
        val res = decode32(input)
        if ((res & 0x00800000) != 0) res | 0xff000000 else res
    }

    /**
     * Decodes a previously encoded 4-byte value from its string representation.
     *
     * @param input a 1-6 byte string, which was the output of a previous encoding call
     * @return the decoded 32 bit value, interpreted as unsigned value
     */
    def decode32(input: String): Int = {
        var res = if (input.head == encTable(ONES)) -1 else 0
        for (byte <- input) res = (res << 6) + dec(byte);
        res
    }

    /**
     * Decodes a previously encoded 6-byte value from its string representation.
     *
     * @param input a 1-8 byte string, which was the output of a previous encoding call
     * @return the decoded 48 bit value, interpreted as unsigned value
     */
    def decode48(input: String): Long = decode64(input) & ((1L << 48) - 1)

    /**
     * Convenience method for signed values; the decoding does not differ from
     * the unsigned one, with the exception of the first 8 bits, which will repeat
     * the sign of the 48 bit number.
     *
     * @param input a 1-8 byte string, which was the output of a previous encoding call
     * @return the decoded 48 bit value, with the highest 8 bit replicating the sign
     */
    def decode48Signed(input: String): Long = {
        val res = decode64(input)
        if ((res & 0x00800000000000L) != 0) res | 0xffff000000000000L else res
    }

    /**
     * Decodes a previously encoded 8-byte value from its string representation.
     *
     * @param input a 1-11 byte string, which was the output of a previous encoding call
     * @return the decoded 64 bit value, interpreted as unsigned value
     */
    def decode64(input: String): Long = {
        var res = if (input.head == encTable(ONES)) -1L else 0L
        for (byte <- input) res = (res << 6) + dec(byte);
        res
    }

    /**
     * Decodes a previously encoded 16-byte value from its string representation.
     *
     * @param input a 1-22 byte string, which was the output of a previous encoding call
     * @return the decoded 128 bit value
     */
    def decode128(input: String): (Long, Long) = {
        var ab = if (input.head == encTable(ONES)) -1L else 0L
        var cd = ab
        for (byte <- input) {
            ab = (ab << 6) + (cd >> 58 & ONES);
            cd = (cd << 6) + dec(byte);
        }
        (ab, cd)
    }
}
