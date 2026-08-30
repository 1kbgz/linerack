import { ChangeEvent, useEffect, useRef, useState } from "react";

export function AudioAudition() {
  const [fileName, setFileName] = useState("");
  const [source, setSource] = useState("");
  const inputRef = useRef<HTMLInputElement>(null);
  const sourceRef = useRef("");

  useEffect(
    () => () => {
      if (sourceRef.current) URL.revokeObjectURL(sourceRef.current);
    },
    [],
  );

  const chooseFile = (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;
    if (sourceRef.current) URL.revokeObjectURL(sourceRef.current);
    const nextSource = URL.createObjectURL(file);
    sourceRef.current = nextSource;
    setSource(nextSource);
    setFileName(file.name);
  };

  return (
    <div className="audition-content">
      <p className="audition-copy">
        Select LineRack as your computer’s sound output, then choose a local song, movie clip, or
        podcast. The file stays on this computer.
      </p>
      <div className="audition-controls">
        <input accept="audio/*,video/*" hidden onChange={chooseFile} ref={inputRef} type="file" />
        <button className="secondary" onClick={() => inputRef.current?.click()}>
          Choose audio file
        </button>
        <span>{fileName || "No file selected"}</span>
      </div>
      {source && <audio aria-label={`Audition ${fileName}`} controls key={source} src={source} />}
    </div>
  );
}
