#include <stdio.h>
#include <stddef.h>

typedef struct {
  const char *title;
  const char *summary;
  const char *evidence_target;
  const char *tags[8];
  size_t tag_count;
} project_profile_t;

static const project_profile_t profile = {
  "Production Flash and Test Rig",
  "Repeatable board flashing, update-cycle validation, and long-run soak checks for deployment confidence.",
  "Clear pass/fail evidence for firmware updates, board bring-up, and field acceptance.",
  {
  "SWUpdate",
  "Yocto",
  "Shell",
  "QA logs",
  "Hardware lab"
  },
  5u
};

int main(void) {
  printf("%s\n", profile.title);
  printf("Summary: %s\n", profile.summary);
  printf("Evidence target: %s\n", profile.evidence_target);
  printf("Stack:");

  for (size_t index = 0; index < profile.tag_count; ++index) {
    printf(" %s%s", profile.tags[index], index + 1u == profile.tag_count ? "" : ",");
  }

  printf("\n");
  return 0;
}
