#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_VERTEX_CNT  60000
#define MAX_INDEX_CNT   90000
#define LINE_SIZE       256

typedef struct smesh {
  uint32_t  vertex_cnt;
  float     *vertices;
  uint32_t  normal_cnt;
  float     *normals;
  uint32_t  uv_cnt;
  float     *uvs;
  uint32_t  index_cnt;
  uint32_t  *indices;
} smesh;

void smesh_init(smesh *m, uint32_t vertex_cnt, uint32_t normal_cnt, uint32_t uv_cnt, uint32_t index_cnt)
{
  m->vertices = malloc(vertex_cnt * 3 * sizeof(float));
  m->normals = malloc(normal_cnt * 3 * sizeof(float));
  m->uvs = malloc(uv_cnt * 2 * sizeof(float));
  m->indices = malloc(index_cnt * 9 * sizeof(uint32_t));
}

void smesh_release(smesh *m)
{
  free(m->indices);
  free(m->uvs);
  free(m->normals);
  free(m->vertices);
}

uint32_t smesh_read_obj(smesh *m, const char *path) 
{
  m->vertex_cnt = 0;
  m->normal_cnt = 0;
  m->uv_cnt     = 0;
  m->index_cnt  = 0;

  FILE *f = fopen(path, "r");
  if(!f) {
    printf("Failed to open file: %s", path);
    return 1;
  }

  while(!feof(f)) {
    char line[LINE_SIZE] = { 0 };
    fgets(line, LINE_SIZE - 1, f);

    if(line == strstr(line, "vt ")) {
			sscanf(line + 3, "%f %f", &m->uvs[m->uv_cnt * 2], &m->uvs[m->uv_cnt * 2 + 1]);
      m->uv_cnt++;
    } else if(line == strstr(line, "vn ")) {
			sscanf(line + 3, "%f %f %f", &m->normals[m->normal_cnt * 3], &m->normals[m->normal_cnt * 3 + 1], &m->normals[m->normal_cnt * 3 + 2]);
      m->normal_cnt++;
    } else if(line[0] == 'v') {
			sscanf(line + 2, "%f %f %f", &m->vertices[m->vertex_cnt * 3], &m->vertices[m->vertex_cnt * 3 + 1], &m->vertices[m->vertex_cnt * 3 + 2]);
      m->vertex_cnt++;
    }

    if(line[0] == 'f') {
      if(m->vertex_cnt == 0) {
        printf("Expected .obj file with vertices. Only normals and uvs are optional.");
        return 1;
      }

      if(m->uv_cnt > 0 && m->normal_cnt > 0)
        sscanf(line + 2, "%u/%u/%u %u/%u/%u %u/%u/%u",
            &m->indices[m->index_cnt * 9 + 0], &m->indices[m->index_cnt * 9 + 1], &m->indices[m->index_cnt * 9 + 2],
            &m->indices[m->index_cnt * 9 + 3], &m->indices[m->index_cnt * 9 + 4], &m->indices[m->index_cnt * 9 + 5],
            &m->indices[m->index_cnt * 9 + 6], &m->indices[m->index_cnt * 9 + 7], &m->indices[m->index_cnt * 9 + 8]);
      else if(m->uv_cnt + m->normal_cnt > 0)
        sscanf(line + 2, "%u/%u %u/%u %u/%u",
            &m->indices[m->index_cnt * 6 + 0], &m->indices[m->index_cnt * 6 + 1],
            &m->indices[m->index_cnt * 6 + 2], &m->indices[m->index_cnt * 6 + 3],
            &m->indices[m->index_cnt * 6 + 4], &m->indices[m->index_cnt * 6 + 5]);
      else
        sscanf(line + 2, "%u %u %u",
            &m->indices[m->index_cnt * 3 + 0], &m->indices[m->index_cnt * 3 + 1], &m->indices[m->index_cnt * 3 + 2]);

      m->index_cnt++;
    }
  }

  uint32_t items = 3 * (1 + (m->uv_cnt > 0 ? 1 : 0) + (m->normal_cnt > 0 ? 1 : 0));
  for(uint32_t j=0; j<m->index_cnt; j++)
    for(uint32_t i=0; i<items; i++)
      m->indices[items * j + i]--;

  fclose(f);

  printf("Read %u vertices, %u normals, %u uvs, %u indices from '%s'.\n",
      m->vertex_cnt, m->normal_cnt, m->uv_cnt, m->index_cnt, path);
 
  return 0;
}

uint32_t smesh_write_bin(const char *path, const smesh *m)
{
  FILE *f = fopen(path, "wb");
  if(!f) {
    printf("Failed to open file: %s", path);
    return 1;
  }

  fwrite(&m->vertex_cnt, sizeof(m->vertex_cnt), 1, f);
  fwrite(&m->normal_cnt, sizeof(m->normal_cnt), 1, f);
  fwrite(&m->uv_cnt, sizeof(m->uv_cnt), 1, f);
  fwrite(&m->index_cnt, sizeof(m->index_cnt), 1, f);

  fwrite(m->vertices, sizeof(*m->vertices), m->vertex_cnt * 3, f);
  fwrite(m->normals, sizeof(*m->normals), m->normal_cnt * 3, f);
  fwrite(m->uvs, sizeof(*m->uvs), m->uv_cnt * 2, f);
  
  uint32_t items = 3 * (1 + (m->uv_cnt > 0 ? 1 : 0) + (m->normal_cnt > 0 ? 1 : 0));
  fwrite(m->indices, sizeof(*m->indices), m->index_cnt * items, f);

  fclose(f);

  printf("Wrote '%s'. Filesize should be %ld bytes.\n", path,
      4 * sizeof(int) + m->vertex_cnt * 3 * sizeof(*m->vertices) + m->normal_cnt * 3 * sizeof(*m->normals) +
      m->uv_cnt * 2 * sizeof(*m->uvs) + m->index_cnt * items * sizeof(*m->indices));
  
  return 0;
}

int main(int argc, char *argv[])
{
  if(argc < 3) {
    printf("Usage: %s input.obj output.bin\n", argv[0]);
    return 1;
  }

  smesh m;
  smesh_init(&m, MAX_VERTEX_CNT, MAX_VERTEX_CNT, MAX_VERTEX_CNT, MAX_INDEX_CNT);

  smesh_read_obj(&m, argv[1]); 
  smesh_write_bin(argv[2], &m);

  smesh_release(&m);

  return 0;
}
