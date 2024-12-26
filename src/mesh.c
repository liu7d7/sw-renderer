#define _CRT_SECURE_NO_WARNINGS

#include "mesh.h"
#include <stdio.h>
#include <string.h>
#include "err.h"
#include "macro.h"

struct mats {
  mat_t *mats;
  int n_mats;
};

struct mats
_parse_mats(char const *file) {
  FILE *f;
  fopen_s(&f, file, "r");
  if (!f) {
    err("failed to open file: %s\n", file);
  }

  mat_t *mats = malloc(4 * sizeof(mat_t));
  int n_mats = 0, c_mats = 4;

  bool first_run = true;
  mat_t current_mat;

  char buf[128];
  while (fgets(buf, 128, f)) {
    if (buf[0] == '#' || buf[0] == '\0') continue;

    if (buf[0] == 'n' && buf[1] == 'e') {
      if (!first_run) {
        check_vec_resize(mats);
        mats[n_mats++] = current_mat;
      }

      first_run = false;

      char name[128];
      sscanf(buf, "%*s %s", name);

      current_mat = (mat_t){
        .name = strdup(name)
      };
    }

    if (buf[0] == 'N' && buf[1] == 's') {
      sscanf(buf, "%*s %f", &current_mat.ns);
    }

    if (buf[0] == 'K') {
      if (buf[1] == 'a') {
        sscanf(
            buf,
            "%*s %f %f %f", 
            &current_mat.ka.x, 
            &current_mat.ka.y, 
            &current_mat.ka.z);
      } else if (buf[1] == 'd') {
        sscanf(
            buf,
            "%*s %f %f %f", 
            &current_mat.kd.x, 
            &current_mat.kd.y, 
            &current_mat.kd.z);
      } else if (buf[1] == 's') {
        sscanf(
            buf,
            "%*s %f %f %f", 
            &current_mat.ks.x, 
            &current_mat.ks.y, 
            &current_mat.ks.z);
      }
    }
  }

  mats[n_mats++] = current_mat;

  return (struct mats){
    .mats = mats,
    .n_mats = n_mats
  };
}

mesh_t
mesh_new(char const *file) {
  FILE *f;
  fopen_s(&f, file, "r");
  if (!f) {
    err("failed to open file: %s\n", file);
  }

  int last_slash;
  for (last_slash = strlen(file) - 1; last_slash >= 0; last_slash--) {
    if (file[last_slash] == '/') break;
  }

  last_slash++;

  char *path = memcpy(malloc(last_slash + 1), file, last_slash);
  path[last_slash] = '\0';

  mat_t *mtls = malloc(4 * sizeof(mat_t));
  int n_mtls = 0, c_mtls = 4;

  vert_t *verts = malloc(4 * sizeof(vert_t));
  int n_verts = 0, c_verts = 4;

  v3_t *pos = malloc(4 * sizeof(v3_t));
  int n_pos = 0, c_pos = 4;

  v3_t *norm = malloc(4 * sizeof(v3_t));
  int n_norm = 0, c_norm = 4;

  v2_t *tex = malloc(4 * sizeof(v2_t));
  int n_tex = 0, c_tex = 4;

  int mtl = 0;

  char buf[128];
  while (fgets(buf, 128, f)) {
    if (buf[0] == '#' || buf[0] == '\0') continue;

    if (buf[0] == 'u' && buf[1] == 's') {
      char mtl_name[64];
      sscanf(buf, "%*s %63s", mtl_name);
      for (int i = 0; i < n_mtls; i++) {
        if (strcmp(mtls[i].name, mtl_name) == 0) {
          mtl = i;
          break;
        }
      }
    }

    if (buf[0] == 'm' && buf[1] == 't') {
      char mtl_file_name[64];
      sscanf(buf, "%*s %63s", mtl_file_name);
      char mtl_path[256] = {0};
      strcat_s(mtl_path, sizeof(mtl_path), path);
      strcat_s(mtl_path, sizeof(mtl_path), mtl_file_name);
      struct mats mats = _parse_mats(mtl_path);
      check_vec_resize_n(mtls, mats.n_mats);
      for (int i = 0; i < mats.n_mats; i++) {
        mtls[n_mtls++] = mats.mats[i];
      }
    }

    if (buf[0] == 'v' && buf[1] == ' ') {
      check_vec_resize(pos);
      v3_t p;
      sscanf(buf, "%*s %f %f %f", &p.x, &p.y, &p.z);
      pos[n_pos++] = p;
    }

    if (buf[0] == 'v' && buf[1] == 'n') {
      check_vec_resize(norm);
      v3_t p;
      sscanf(buf, "%*s %f %f %f", &p.x, &p.y, &p.z);
      norm[n_norm++] = p;
    }

    if (buf[0] == 'v' && buf[1] == 't') {
      check_vec_resize(tex);
      v2_t p;
      sscanf(buf, "%*s %f %f", &p.x, &p.y);
      tex[n_tex++] = p;
    }

    if (buf[0] == 'f' && buf[1] == ' ') {
      check_vec_resize_n(verts, 3);
      int _0, _1, _2, _3, _4, _5, _6, _7, _8;
      sscanf(
          buf, 
          "%*s %d/%d/%d %d/%d/%d %d/%d/%d", 
          &_0, &_1, &_2, &_3, &_4, &_5, &_6, &_7, &_8);

      _0--, _1--, _2--, _3--, _4--, _5--, _6--, _7--, _8--;

      verts[n_verts++] = (vert_t){
        pos[_0],
        norm[_2],
        tex[_1],
        mtl
      };

      verts[n_verts++] = (vert_t){
        pos[_3],
        norm[_5],
        tex[_4],
        mtl
      };

       verts[n_verts++] = (vert_t){
        pos[_6],
        norm[_8],
        tex[_7],
        mtl
      };
    }
  }

  fclose(f);

  print("finished parsing %s\n", file);

  return (mesh_t) {
    .verts = verts,
    .n_verts = n_verts,
    .mats = mtls,
    .n_mats = n_mtls
  };
}
