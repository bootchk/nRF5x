/*
 * Types used by radio driver
 */

/*
 * Pointer to unsigned bytes writeable concurrently by device and mcu.
 * The pointed-to uint8_t is volatile.
 *
 * A BufferPointer points into memory.
 * You pass a BufferPointer to the radio device.
 * Both the radio driver (the mcu) and the radio device access the buffer concurrently.
 * I.E. there are two separate threads/processors accessing buffer.
 */
typedef volatile uint8_t * RadioBufferPointer;
