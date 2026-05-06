/*
 * inventory.c  –  C data layer
 * Binary file storage with fread/fwrite/fseek.
 * All public functions are declared in include/inventory.h.
 */

#include <stdio.h>
#include <string.h>
#include "inventory.h"

/* ------------------------------------------------------------------ helpers */

/* Open the database file.  mode follows fopen conventions. */
static FILE *open_db(const char *mode)
{
    return fopen(DB_FILE, mode);
}

/*
 * Find the file offset of the record whose id matches `target_id`.
 * Returns the byte offset on success, -1L if not found.
 * The file must already be open and positioned anywhere.
 */
static long find_offset(FILE *fp, int target_id)
{
    Item   tmp;
    size_t n;
    long   offset = 0L;

    rewind(fp);
    while ((n = fread(&tmp, sizeof(Item), 1, fp)) == 1) {
        if (tmp.id == target_id)
            return offset;
        offset += (long)sizeof(Item);
    }
    return -1L;
}

/* ------------------------------------------------------------------ public API */

/*
 * add_item – append a new record after verifying the ID is unique.
 * Returns 1 on success, 0 if the ID already exists or I/O fails.
 */
int add_item(const Item *item)
{
    FILE *fp;
    Item  tmp;

    if (!item || item->id <= 0)
        return 0;

    /* Check for duplicate ID (including soft-deleted records). */
    fp = open_db("rb");
    if (fp) {
        while (fread(&tmp, sizeof(Item), 1, fp) == 1) {
            if (tmp.id == item->id) {
                fclose(fp);
                return 0;   /* duplicate */
            }
        }
        fclose(fp);
    }

    /* Append the new record. */
    fp = open_db("ab");
    if (!fp) {
        /* File does not exist yet – create it. */
        fp = fopen(DB_FILE, "wb");
        if (!fp) return 0;
    }

    if (fwrite(item, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

/*
 * get_item – read one active record by ID into *out.
 * Returns 1 on success, 0 if not found or soft-deleted.
 */
int get_item(int id, Item *out)
{
    FILE *fp;
    long  offset;

    if (!out || id <= 0)
        return 0;

    fp = open_db("rb");
    if (!fp) return 0;

    offset = find_offset(fp, id);
    if (offset < 0L) {
        fclose(fp);
        return 0;
    }

    fseek(fp, offset, SEEK_SET);
    if (fread(out, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fclose(fp);

    /* Treat soft-deleted records as non-existent. */
    return (out->is_deleted == 0) ? 1 : 0;
}

/*
 * update_item – overwrite the fields of an existing active record.
 * The id field in *updated is ignored; the record is located by `id`.
 * Returns 1 on success, 0 on failure.
 */
int update_item(int id, const Item *updated)
{
    FILE *fp;
    long  offset;
    Item  existing;
    Item  new_rec;

    if (!updated || id <= 0)
        return 0;

    fp = open_db("r+b");
    if (!fp) return 0;

    offset = find_offset(fp, id);
    if (offset < 0L) {
        fclose(fp);
        return 0;
    }

    /* Read existing record first to check is_deleted. */
    fseek(fp, offset, SEEK_SET);
    if (fread(&existing, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    if (existing.is_deleted) {
        fclose(fp);
        return 0;
    }

    /* Build updated record preserving the original ID. */
    new_rec            = *updated;
    new_rec.id         = id;
    new_rec.is_deleted = 0;

    /* Seek back and overwrite. */
    fseek(fp, offset, SEEK_SET);
    if (fwrite(&new_rec, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fflush(fp);
    fclose(fp);
    return 1;
}

/*
 * delete_item – soft delete: sets is_deleted = 1 in the file.
 * Returns 1 on success, 0 on failure.
 */
int delete_item(int id)
{
    FILE *fp;
    long  offset;
    Item  tmp;

    if (id <= 0)
        return 0;

    fp = open_db("r+b");
    if (!fp) return 0;

    offset = find_offset(fp, id);
    if (offset < 0L) {
        fclose(fp);
        return 0;
    }

    fseek(fp, offset, SEEK_SET);
    if (fread(&tmp, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    if (tmp.is_deleted) {
        fclose(fp);
        return 0;   /* already deleted */
    }

    tmp.is_deleted = 1;
    fseek(fp, offset, SEEK_SET);
    if (fwrite(&tmp, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fflush(fp);
    fclose(fp);
    return 1;
}

/*
 * list_items – fill buffer with up to max_items active records.
 * Returns the count of items copied.
 */
int list_items(Item *buffer, int max_items)
{
    FILE *fp;
    Item  tmp;
    int   count = 0;

    if (!buffer || max_items <= 0)
        return 0;

    fp = open_db("rb");
    if (!fp) return 0;

    while (count < max_items && fread(&tmp, sizeof(Item), 1, fp) == 1) {
        if (!tmp.is_deleted) {
            buffer[count++] = tmp;
        }
    }
    fclose(fp);
    return count;
}
