# Workspace

## Overview

pnpm workspace monorepo using TypeScript. Each package manages its own dependencies.

## Stack

- **Monorepo tool**: pnpm workspaces
- **Node.js version**: 24
- **Package manager**: pnpm
- **TypeScript version**: 5.9
- **API framework**: Express 5
- **Database**: PostgreSQL + Drizzle ORM
- **Validation**: Zod (`zod/v4`), `drizzle-zod`
- **API codegen**: Orval (from OpenAPI spec)
- **Build**: esbuild (CJS bundle)

## Key Commands

- `pnpm run typecheck` — full typecheck across all packages
- `pnpm run build` — typecheck + build all packages
- `pnpm --filter @workspace/api-spec run codegen` — regenerate API hooks and Zod schemas from OpenAPI spec
- `pnpm --filter @workspace/db run push` — push DB schema changes (dev only)
- `pnpm --filter @workspace/api-server run dev` — run API server locally

See the `pnpm-workspace` skill for workspace structure, TypeScript setup, and package details.

## Projects

### Basic Bank Account System (`artifacts/bank-system`)
- **Frontend**: React + Vite at `/` — HTML+CSS styled banking UI
- **Backend**: Express API at `/api/accounts` — PostgreSQL storage
- **C Source**: `bank.c` at project root — standalone C program (for local/offline use)
- **Pages**: All Accounts, Create Account, Deposit, Withdraw, Check Balance

## Database Schema

### accounts table
- `account_number` (varchar PK) — 6-digit unique account number
- `name` (varchar) — account holder name
- `balance` (double precision) — current balance
- `created_at` (timestamp) — creation time
