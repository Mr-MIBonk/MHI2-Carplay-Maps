static AltFrameRing *create_forward_ring(void)
{
    AltFrameRing *ring;
    uint32_t generation = 1;
    unsigned int i;
    int fd = open(ALT_FRAME_RING_PATH, O_RDWR | O_CREAT, 0666);
    if (fd < 0) return NULL;
    if (ftruncate(fd, (off_t)sizeof(*ring)) != 0) {
        close(fd);
        return NULL;
    }
    ring = (AltFrameRing *)mmap(NULL, sizeof(*ring), PROT_READ | PROT_WRITE,
                               MAP_SHARED, fd, 0);
    close(fd);
    if (ring == MAP_FAILED) return NULL;
    if (ring->magic == ALT_FRAME_RING_MAGIC &&
        ring->version == ALT_FRAME_RING_VERSION)
        generation = ring->generation + 1U;
    ring->magic = 0;
    __sync_synchronize();
    ring->version = ALT_FRAME_RING_VERSION;
    ring->generation = generation ? generation : 1U;
    ring->write_sequence = 0;
    for (i = 0; i < ALT_FRAME_RING_SLOT_COUNT; ++i)
        ring->slots[i].committed_sequence = 0;
    __sync_synchronize();
    ring->magic = ALT_FRAME_RING_MAGIC;
    return ring;
}

static int forward_ring_message(AltFrameRing *ring, uint16_t type,
                                uint32_t source_sequence, uint32_t flags,
                                uint64_t media_timestamp_ns,
                                const uint8_t *data, size_t size)
{
    AltFrameRingSlot *slot;
    uint32_t sequence;
    if (!ring || !data || size == 0 || size > ALT_FRAME_RING_SLOT_DATA_MAX)
        return -1;
    sequence = ring->write_sequence + 1U;
    if (sequence == 0) sequence = 1U;
    slot = &ring->slots[(sequence - 1U) % ALT_FRAME_RING_SLOT_COUNT];
    slot->committed_sequence = 0;
    __sync_synchronize();
    slot->type = type;
    slot->size = (uint32_t)size;
    slot->flags = flags;
    slot->source_sequence = source_sequence;
    slot->media_timestamp_lo = (uint32_t)media_timestamp_ns;
    slot->media_timestamp_hi = (uint32_t)(media_timestamp_ns >> 32);
    memcpy(slot->data, data, size);
    __sync_synchronize();
    slot->committed_sequence = sequence;
    __sync_synchronize();
    ring->write_sequence = sequence;
    return 1;
}
