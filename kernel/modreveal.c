#include <linux/kprobes.h>
#include <net/genetlink.h>

enum {
  MODREVEAL_A_UNSPEC,
  MODREVEAL_A_STR_0,
  __MODREVEAL_A_MAX,
};
#define MODREVEAL_A_MAX (__MODREVEAL_A_MAX - 1)

enum {
  MODREVEAL_C_UNSPEC,
  MODREVEAL_C_GET_MODULES,
  __MODREVEAL_C_MAX,
};
#define MODREVEAL_C_MAX (__MODREVEAL_C_MAX - 1)

static struct nla_policy modreveal_genl_policy[MODREVEAL_A_MAX + 1] = {
  [MODREVEAL_A_STR_0] = { .type = NLA_NUL_STRING },
};

static int get_modules(struct sk_buff *, struct genl_info *);

static struct genl_ops modreveal_ops[] = {
  {
    .cmd = MODREVEAL_C_GET_MODULES,
    .flags = 0,
    .policy = modreveal_genl_policy,
    .doit = get_modules,
  },
};

static struct genl_family modreveal_genl_family = {
  .id = 0x0,
  .hdrsize = 0,
  .name = "modreveal",
  .version = 1,
  .maxattr = MODREVEAL_A_MAX,
  .ops = modreveal_ops,
  .n_ops = MODREVEAL_C_MAX,
};

typedef void *(*kallsyms_lookup_name_t)(const char *name);
static kallsyms_lookup_name_t lookup;

static int get_modules(struct sk_buff *skb, struct genl_info *info) {
  struct kset *mod_kset = lookup("module_kset");
  struct kobject *cur, *tmp;
  char *buf = kzalloc(128000, GFP_KERNEL);
  unsigned int buf_p = 0;

  list_for_each_entry_safe(cur, tmp, &mod_kset->list, entry) {
    if (!kobject_name(tmp))
      break;

    struct module_kobject *kobj = container_of(tmp, struct module_kobject, kobj);

    if (kobj && kobj->mod) {
      strcat(buf + buf_p, kobj->mod->name);
      buf_p += strlen(kobj->mod->name) + 1;
      strcat(buf, " ");
    }
  }

  struct sk_buff *reply_skb = genlmsg_new(buf_p, GFP_KERNEL);
  if (reply_skb == NULL) {
    pr_err("An error occurred in %s()\n", __func__);
    kfree(buf);
    return -ENOMEM;
  }

  void *msg_head = genlmsg_put(reply_skb, info->snd_portid, info->snd_seq + 1, &modreveal_genl_family, 0, MODREVEAL_C_GET_MODULES);
  if (msg_head == NULL) {
    pr_err("An error occurred in %s()\n", __func__);
    kfree(buf);
    return -ENOMEM;
  }

  int rc = nla_put(reply_skb, MODREVEAL_A_STR_0, buf_p, buf);
  if (rc != 0) {
    pr_err("An error occurred in %s()\n", __func__);
    kfree(buf);
    return -rc;
  }

  genlmsg_end(reply_skb, msg_head);
  rc = genlmsg_reply(reply_skb, info);
  if (rc != 0) {
    pr_err("An error occurred in %s()\n", __func__);
    kfree(buf);
    return -rc;
  }

  kfree(buf);
  return 0;
}

void resolve_kallsyms_lookup_name(void) {
  static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
  };
  register_kprobe(&kp);
  lookup = (kallsyms_lookup_name_t)kp.addr;
  unregister_kprobe(&kp);
}

static int __init modinit(void) {
  int rc = genl_register_family(&modreveal_genl_family);
  if (rc != 0) {
    pr_err("%s\n", "Couldn't register generic netlink family");
    return 1;
  }

  resolve_kallsyms_lookup_name();

  return 0;
}

static void __exit modexit(void) {
  int rc = genl_unregister_family(&modreveal_genl_family);
  if (rc != 0) pr_err("%s\n", "Failed to unregister netlink family");
}

module_init(modinit);
module_exit(modexit);
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.0");
