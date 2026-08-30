import { describe, expect, it } from "vitest";
import appSource from "./App.tsx?raw";

describe("unapplied-change navigation warning", () => {
  it("registers beforeunload only while the draft is dirty and removes it afterward", () => {
    expect(appSource).toContain("if (!dirty) return;");
    expect(appSource).toContain('window.addEventListener("beforeunload", warnBeforeUnload)');
    expect(appSource).toContain('window.removeEventListener("beforeunload", warnBeforeUnload)');
    expect(appSource).toContain("event.preventDefault()");
    expect(appSource).toContain("event.returnValue = true");
  });

  it("preserves newer drafts across connection and write completion", () => {
    expect(appSource).toContain("const preserveDraft = dirtyRef.current");
    expect(appSource).toContain("if (!preserveDraft)");
    expect(appSource).toContain("draftRevision.current === revision");
    expect(appSource).toContain("if (applyingRef.current) return");
  });
});
