#include <stdint.h>
#include <string.h>

#include "mmu.h"
#include "pager.h"
#include "ram.h"
#include "task.h"

static uint8_t *g_ram_base = NULL;
static tRam *g_ram_state = NULL;
static tPageTableEntry *g_active_page_table = NULL;
static tTaskMgr *g_task_mgr = NULL;
static uint16_t g_task_mgr_frame = 0;
static uint16_t g_task_mgr_frames = 0;

static int is_power_of_two(uint16_t value)
{
    return value && ((value & (value - 1)) == 0);
}

static uint16_t get_frame_count(void)
{
    if (g_ram_state == NULL || g_ram_state->page_size == 0)
    {
        return 0;
    }
    return (uint16_t)(g_ram_state->size / g_ram_state->page_size);
}

static int frame_is_used(uint16_t frame_id)
{
    const uint8_t *bitmap = g_ram_state->bitmap;
    return (bitmap[frame_id / 8] >> (frame_id % 8)) & 0x1;
}

static void mark_frame(uint16_t frame_id, int used)
{
    uint8_t *bitmap = g_ram_state->bitmap;
    uint8_t mask = (uint8_t)(1u << (frame_id % 8));
    if (used)
    {
        bitmap[frame_id / 8] |= mask;
    }
    else
    {
        bitmap[frame_id / 8] &= (uint8_t)~mask;
    }
}

int init_ram(void *memory, uint16_t size, uint8_t page_size)
{
    if (memory == NULL)
    {
        return -3;
    }

    if (!is_power_of_two(size) || size == 0)
    {
        return -1;
    }

    if (!is_power_of_two(page_size) || page_size == 0 || page_size > size)
    {
        return -2;
    }

    uint8_t *base = (uint8_t *)memory;
    for (uint16_t i = 0; i < size; ++i)
    {
        if (base[i] != 0)
        {
            return -3;
        }
    }

    uint16_t frame_count = (uint16_t)(size / page_size);
    uint16_t bitmap_size = (uint16_t)((frame_count + 7u) / 8u);
    uint32_t metadata_size = (uint32_t)sizeof(tRam) + bitmap_size;
    uint16_t frames_needed = (uint16_t)((metadata_size + page_size - 1u) / page_size);

    if (frames_needed > frame_count)
    {
        return -4;
    }

    g_ram_base = base;
    g_ram_state = (tRam *)base;
    g_ram_state->size = size;
    g_ram_state->page_size = page_size;
    g_ram_state->bitmap = g_ram_base + sizeof(tRam);

    memset(g_ram_state->bitmap, 0, bitmap_size);

    for (uint16_t i = 0; i < frames_needed; ++i)
    {
        mark_frame(i, 1);
    }

    g_active_page_table = NULL;
    g_task_mgr = NULL;
    g_task_mgr_frame = 0;
    g_task_mgr_frames = 0;

    return frame_count;
}

void destroy_ram()
{
    g_ram_base = NULL;
    g_ram_state = NULL;
    g_active_page_table = NULL;
    g_task_mgr = NULL;
    g_task_mgr_frame = 0;
    g_task_mgr_frames = 0;
}

int falloc(uint16_t *frame_id, uint16_t number)
{
    if (g_ram_state == NULL)
    {
        return -1;
    }

    if (frame_id == NULL || number == 0)
    {
        return -2;
    }

    uint16_t frame_count = get_frame_count();
    if (frame_count == 0 || number > frame_count)
    {
        return -1;
    }

    uint16_t start = 0;
    while (start <= frame_count - number)
    {
        uint16_t i = 0;
        for (; i < number; ++i)
        {
            if (frame_is_used((uint16_t)(start + i)))
            {
                break;
            }
        }
        if (i == number)
        {
            for (uint16_t j = 0; j < number; ++j)
            {
                mark_frame((uint16_t)(start + j), 1);
            }
            *frame_id = start;
            return 0;
        }
        start = (uint16_t)(start + i + 1);
    }

    return -1;
}

void ffree(uint16_t frame_id, uint16_t number)
{
    if (g_ram_state == NULL)
    {
        return;
    }

    uint16_t frame_count = get_frame_count();
    if (frame_id >= frame_count || number == 0)
    {
        return;
    }

    uint16_t limit = (uint16_t)((frame_id + number > frame_count) ? (frame_count - frame_id) : number);
    for (uint16_t i = 0; i < limit; ++i)
    {
        mark_frame((uint16_t)(frame_id + i), 0);
    }
}

const tRam *get_ram_state()
{
    return g_ram_state;
}

int init_taskMgr()
{
    if (g_ram_state == NULL || g_task_mgr != NULL)
    {
        return -1;
    }

    uint8_t page_size = g_ram_state->page_size;
    if (page_size == 0)
    {
        return -1;
    }

    uint32_t bytes_needed = (uint32_t)sizeof(tTaskMgr);
    g_task_mgr_frames = (uint16_t)((bytes_needed + page_size - 1u) / page_size);

    if (falloc(&g_task_mgr_frame, g_task_mgr_frames) != 0)
    {
        g_task_mgr_frames = 0;
        return -1;
    }

    g_task_mgr = (tTaskMgr *)(g_ram_base + (g_task_mgr_frame * page_size));
    memset(g_task_mgr, 0, sizeof(tTaskMgr));

    for (int i = 0; i < TASK_TABLE_SIZE; ++i)
    {
        g_task_mgr->tasks[i].pid = -1;
    }

    return 0;
}

void destroy_taskMgr()
{
    if (g_task_mgr == NULL)
    {
        return;
    }

    for (int i = 0; i < TASK_TABLE_SIZE; ++i)
    {
        if (g_task_mgr->tasks[i].pid != -1)
        {
            destroy_task(g_task_mgr->tasks[i].pid);
        }
    }

    ffree(g_task_mgr_frame, g_task_mgr_frames);

    g_task_mgr = NULL;
    g_task_mgr_frame = 0;
    g_task_mgr_frames = 0;
}

int create_task(const tPageTableEntry *page_table, uint8_t max_frames, void *address_space)
{
    if (g_ram_state == NULL || g_task_mgr == NULL)
    {
        return -3;
    }

    if (page_table == NULL || address_space == NULL)
    {
        return -2;
    }

    int free_index = -1;
    for (int i = 0; i < TASK_TABLE_SIZE; ++i)
    {
        if (g_task_mgr->tasks[i].pid == -1)
        {
            free_index = i;
            break;
        }
    }

    if (free_index == -1)
    {
        return -1;
    }

    tTaskStruct *task = &g_task_mgr->tasks[free_index];
    task->pid = free_index;
    task->max_frames = max_frames;
    task->address_space = address_space;
    memcpy(task->page_table, page_table, sizeof(task->page_table));

    return task->pid;
}

int destroy_task(int pid)
{
    tTaskStruct *task = get_task_struct(pid);
    if (task == NULL)
    {
        return -1;
    }

    for (int i = 0; i < PAGE_TABLE_SIZE; ++i)
    {
        if (task->page_table[i].p_bit)
        {
            ffree(task->page_table[i].frame_id, 1);
        }
        memset(&task->page_table[i], 0, sizeof(task->page_table[i]));
    }

    task->pid = -1;
    task->max_frames = 0;
    task->address_space = NULL;

    return 0;
}

const tTaskMgr *get_task_mgr()
{
    return g_task_mgr;
}

tTaskStruct *get_task_struct(int pid)
{
    if (g_task_mgr == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < TASK_TABLE_SIZE; ++i)
    {
        if (g_task_mgr->tasks[i].pid == pid)
        {
            return &g_task_mgr->tasks[i];
        }
    }

    return NULL;
}

void set_page_table(tPageTableEntry *page_table)
{
    g_active_page_table = page_table;
}

int get_physical_address(uint16_t virtual_address, uint16_t *physical_address)
{
    if (physical_address == NULL)
    {
        return -3;
    }

    if (g_ram_state == NULL)
    {
        return -5;
    }

    if (g_active_page_table == NULL)
    {
        return -4;
    }

    uint8_t page_size = g_ram_state->page_size;
    uint16_t page_index = (uint16_t)(virtual_address / page_size);
    uint16_t offset = (uint16_t)(virtual_address % page_size);

    if (page_index >= PAGE_TABLE_SIZE)
    {
        return -2;
    }

    tPageTableEntry *entry = &g_active_page_table[page_index];

    // Check if page is present in RAM
    if (entry->p_bit == 0)
    {
        return -1;
    }

    *physical_address = (uint16_t)(entry->frame_id * page_size + offset);
    return 0;
}

int fetch_instruction(uint16_t virtual_address, uint8_t *data)
{
    if (g_ram_state == NULL)
    {
        return -5;
    }
    if (g_active_page_table == NULL)
    {
        return -4;
    }

    uint8_t page_size = g_ram_state->page_size;
    uint16_t page_index = (uint16_t)(virtual_address / page_size);
    uint16_t offset = (uint16_t)(virtual_address % page_size);

    if (page_index >= PAGE_TABLE_SIZE)
    {
        return -2;
    }

    tPageTableEntry *entry = &g_active_page_table[page_index];

    // Check if page is present in RAM
    if (entry->p_bit == 0)
    {
        return -1;
    }

    // Check execute permission - segmentation fault if page not accessible, access violation if no x permission
    if (entry->r == 0 && entry->w == 0 && entry->x == 0)
    {
        return -2;
    }
    if (entry->x == 0)
    {
        return -3;
    }

    uint16_t physical = (uint16_t)(entry->frame_id * page_size + offset);
    entry->r_bit = 1;
    *data = g_ram_base[physical];
    return 0;
}

int load_data(uint16_t virtual_address, uint8_t *data)
{
    if (g_ram_state == NULL)
    {
        return -5;
    }
    if (g_active_page_table == NULL)
    {
        return -4;
    }

    uint8_t page_size = g_ram_state->page_size;
    uint16_t page_index = (uint16_t)(virtual_address / page_size);
    uint16_t offset = (uint16_t)(virtual_address % page_size);

    if (page_index >= PAGE_TABLE_SIZE)
    {
        return -2;
    }

    tPageTableEntry *entry = &g_active_page_table[page_index];

    // Check if page is present in RAM
    if (entry->p_bit == 0)
    {
        return -1;
    }

    // Check read permission - segmentation fault if page not accessible, access violation if no r permission
    if (entry->r == 0 && entry->w == 0 && entry->x == 0)
    {
        return -2;
    }
    if (entry->r == 0)
    {
        return -3;
    }

    uint16_t physical = (uint16_t)(entry->frame_id * page_size + offset);
    entry->r_bit = 1;
    *data = g_ram_base[physical];
    return 0;
}

int store_data(uint16_t virtual_address, uint8_t data)
{
    if (g_ram_state == NULL)
    {
        return -5;
    }
    if (g_active_page_table == NULL)
    {
        return -4;
    }

    uint8_t page_size = g_ram_state->page_size;
    uint16_t page_index = (uint16_t)(virtual_address / page_size);
    uint16_t offset = (uint16_t)(virtual_address % page_size);

    if (page_index >= PAGE_TABLE_SIZE)
    {
        return -2;
    }

    tPageTableEntry *entry = &g_active_page_table[page_index];

    // Check if page is present in RAM
    if (entry->p_bit == 0)
    {
        return -1;
    }

    // Check write permission - segmentation fault if page not accessible, access violation if no w permission
    if (entry->r == 0 && entry->w == 0 && entry->x == 0)
    {
        return -2;
    }
    if (entry->w == 0)
    {
        return -3;
    }

    uint16_t physical = (uint16_t)(entry->frame_id * page_size + offset);
    entry->r_bit = 1;
    entry->m_bit = 1;
    g_ram_base[physical] = data;
    return 0;
}

static void write_back_modified_pages(tTaskStruct *task)
{
    if (task == NULL || g_ram_state == NULL)
    {
        return;
    }

    uint8_t page_size = g_ram_state->page_size;
    uint16_t frame_count = get_frame_count();
    uint8_t *address_space = (uint8_t *)task->address_space;

    for (int i = 0; i < PAGE_TABLE_SIZE; ++i)
    {
        tPageTableEntry *entry = &task->page_table[i];
        if (entry->p_bit && entry->m_bit && entry->frame_id < frame_count && address_space != NULL)
        {
            uint8_t *src = g_ram_base + (entry->frame_id * page_size);
            memcpy(address_space + (i * page_size), src, page_size);
        }
    }
}

int page_fault(int pid, uint16_t virtual_address)
{
    tTaskStruct *task = get_task_struct(pid);
    if (task == NULL)
    {
        return -1;
    }

    if (g_ram_state == NULL)
    {
        return -5;
    }

    if (task->address_space == NULL)
    {
        return -3;
    }

    uint8_t page_size = g_ram_state->page_size;
    uint16_t page_index = (uint16_t)(virtual_address / page_size);

    if (page_index >= PAGE_TABLE_SIZE)
    {
        return -4;
    }

    tPageTableEntry *entry = &task->page_table[page_index];
    if (entry->r == 0 && entry->w == 0 && entry->x == 0)
    {
        return -4;
    }

    if (entry->p_bit)
    {
        return -2;
    }

    uint16_t frame_count = get_frame_count();
    write_back_modified_pages(task);

    int pages_in_ram = 0;
    for (int i = 0; i < PAGE_TABLE_SIZE; ++i)
    {
        if (task->page_table[i].p_bit)
        {
            ++pages_in_ram;
        }
    }

    uint16_t frame_id = 0;
    int need_victim = 0;

    if (task->max_frames > 0 && pages_in_ram >= task->max_frames)
    {
        need_victim = 1;
    }

    if (!need_victim)
    {
        if (falloc(&frame_id, 1) != 0)
        {
            need_victim = 1;
        }
    }

    if (need_victim)
    {
        int victim_index = -1;
        int best_class = 4;

        for (int i = 0; i < PAGE_TABLE_SIZE; ++i)
        {
            tPageTableEntry *candidate = &task->page_table[i];
            if (!candidate->p_bit || candidate->frame_id >= frame_count)
            {
                continue;
            }

            int class_value = (candidate->r_bit ? 2 : 0) + (candidate->m_bit ? 1 : 0);
            if (class_value < best_class)
            {
                best_class = class_value;
                victim_index = i;
                if (best_class == 0)
                {
                    break;
                }
            }
        }

        if (victim_index == -1)
        {
            return -3;
        }

        frame_id = task->page_table[victim_index].frame_id;
        task->page_table[victim_index].p_bit = 0;
    }

    if (frame_id >= frame_count)
    {
        return -3;
    }

    uint8_t *address_space = (uint8_t *)task->address_space;
    uint8_t *destination = g_ram_base + (frame_id * page_size);
    memcpy(destination, address_space + (page_index * page_size), page_size);

    entry->frame_id = frame_id;
    entry->p_bit = 1;

    for (int i = 0; i < PAGE_TABLE_SIZE; ++i)
    {
        task->page_table[i].r_bit = 0;
        task->page_table[i].m_bit = 0;
    }

    return 0;
}
