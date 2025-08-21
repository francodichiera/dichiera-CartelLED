"use client"

import { useEffect, useState } from "react"

export function EnhancedRgbLedIcon() {
  const [colorIndex, setColorIndex] = useState(0)
  const ledColors = ["#ef4444", "#f97316", "#eab308", "#22c55e", "#06b6d4", "#3b82f6", "#8b5cf6", "#ec4899"]

  useEffect(() => {
    const interval = setInterval(() => {
      setColorIndex((prev) => (prev + 1) % ledColors.length)
    }, 200)
    return () => clearInterval(interval)
  }, [ledColors.length])

  return (
    <div className="relative w-12 h-12">
      <div className="grid grid-cols-4 grid-rows-4 gap-0.5 w-full h-full">
        {Array.from({ length: 16 }).map((_, i) => (
          <div
            key={i}
            className="w-2.5 h-2.5 rounded-full led-pulse"
            style={{
              backgroundColor: ledColors[(colorIndex + i) % ledColors.length],
              animationDelay: `${i * 0.1}s`,
              boxShadow: `0 0 8px ${ledColors[(colorIndex + i) % ledColors.length]}`,
            }}
          />
        ))}
      </div>

      <div className="absolute inset-0 rounded-full bg-gradient-to-r from-transparent via-white/10 to-transparent animate-pulse" />
    </div>
  )
}
