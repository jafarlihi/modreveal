#include <stdbool.h>
#include <netlink/socket.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

int run_lsmod(char lines[][512]) {
  FILE *fp;
  char path[512];

  fp = popen("lsmod | awk '{print $1}'", "r");
  if (fp == NULL)
    return -1;

  int count = 0;
  while (fgets(path, sizeof(path), fp) != NULL)
    strcpy(lines[count++], path);

  pclose(fp);
  return count;
}

int main(int argc, char *argv[]) {
  char lsmod[512][512];
  int lsmod_count = run_lsmod(lsmod);

  struct nl_sock *sk = nl_socket_alloc();
  if (!sk) {
    fprintf(stderr, "Failed to alloc nl socket\n");
    exit(EXIT_FAILURE);
  }
  if (genl_connect(sk)) {
    fprintf(stderr, "Failed to connect to genl\n");
    exit(EXIT_FAILURE);
  }
  int family_id = genl_ctrl_resolve(sk, "modreveal");
  if (family_id < 0) {
    fprintf(stderr, "Failed to resolve the netlink family name\n");
    exit(EXIT_FAILURE);
  }

  struct nl_msg *msg = nlmsg_alloc();
  if (!msg) {
    fprintf(stderr, "Failed to allocate nl message\n");
    exit(EXIT_FAILURE);
  }
  if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, family_id, 0, 0, 1, 1)) {
    fprintf(stderr, "Failed to put nl header\n");
    nlmsg_free(msg);
    exit(EXIT_FAILURE);
  }
  if (nl_send_auto(sk, msg) < 0) {
    fprintf(stderr, "Failed to send nl message\n");
    nlmsg_free(msg);
    exit(EXIT_FAILURE);
  }

  struct sockaddr_nl sa;
  unsigned char *buf = calloc(4096, sizeof(unsigned char));
  int recv = nl_recv(sk, &sa, &buf, NULL);

  char *module = strtok(buf + 24, " ");
  while (module != NULL) {
    bool found = false;
    for (int i = 0; i < lsmod_count; i++)
      if (strncmp(lsmod[i], module, strlen(module)) == 0) {
        found = true;
        break;
      }
    if (!found)
      printf("%s\n", module);
    module = strtok(NULL, " ");
  }

  free(buf);
  return 0;
}
