import { ReactNode, useState } from "react";

interface CollapsibleSectionProps {
  badge?: string;
  children: ReactNode;
  className?: string;
  defaultOpen?: boolean;
  hidden?: boolean;
  label: string;
  onOpenChange?: (open: boolean) => void;
  title: string;
}

export function CollapsibleSection({
  badge,
  children,
  className = "",
  defaultOpen = false,
  hidden = false,
  label,
  onOpenChange,
  title,
}: CollapsibleSectionProps) {
  const [open, setOpen] = useState(defaultOpen);

  return (
    <details
      className={`collapsible-panel ${className}`.trim()}
      hidden={hidden}
      onToggle={(event) => {
        setOpen(event.currentTarget.open);
        onOpenChange?.(event.currentTarget.open);
      }}
      open={open}
    >
      <summary>
        <span>
          <span className="section-label">{label}</span>
          <h2>{title}</h2>
        </span>
        <span className="collapsible-summary-meta">
          {badge && <small>{badge}</small>}
          <span aria-hidden="true" className="collapse-icon">
            ▾
          </span>
        </span>
      </summary>
      <div className="collapsible-content">{children}</div>
    </details>
  );
}
