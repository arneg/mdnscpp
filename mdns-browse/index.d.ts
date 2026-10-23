export interface BrowseCriteria {
  type: string;
  protocol: string;
  subtype?: string;
  interfaceIndex?: number;
  ipProtocol?: "ipv4" | "ipv6";
}

export type TxtRecord = {
  key: string;
  value?: string;
};

export type BrowseResult = {
  type: string;
  protocol: string;
  name: string;
  domain: string;
  hostname: string;
  address: string;
  port: number;
  interfaceIndex: number;
  txtRecords: TxtRecord[];
};

export type PlatformType = "dns_sd" | "avahi" | "win32";

export interface PlatformError extends Error {
  code: "ERR_MDNS_PLATFORM";
  platform: PlatformType;
  errno: number;
}

export function find(
  options: BrowseCriteria,
  predicate?: (result: BrowseResult) => boolean,
  signal?: AbortSignal
): Promise<BrowseResult>;

export function startBrowse(
  options: BrowseCriteria,
  callback: (results: BrowseResult[], error: PlatformError | null) => void
): () => void;
