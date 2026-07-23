# SM8850 HWID 副本规则

接着上个窗口继续时，先读 `docs/当前会话交接.md`。

- 本目录是 `BailinT/oppo_oplus_realme_sm8850` 的独立副本，不修改原目录或远程原仓库。
- `hwid/allowlist.txt` 是允许启动内核的完整白名单，一行一个；修改后必须重新构建。
- 白名单外或读取不到 HWID 的设备会在 early boot 阶段 panic，并在 1 秒后重启。
- 未明确要求时不要触发或 cancel Actions；原仓库只作为只读 upstream。
