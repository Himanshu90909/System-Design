# URL Shortener System

This project implements the core low-level design for a URL shortener described in the accompanying system-design brief. It is a dependency-free C++17 reference implementation with generated Base62 short codes, custom aliases, expiration, disabling, click analytics, and thread-safe access.

## Requirements covered

- Create a unique short code for a valid long URL.
- Reserve a user-provided custom alias when it is available.
- Resolve active links quickly and record click events asynchronously in a production deployment.
- Reject disabled and expired links.
- Update expiration and disable links.
- Expose click count and click history for analytics.
- Protect shared state with a reader/writer lock and make ID generation atomic.

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -pthread URLShortenerSystem.cpp -o url-shortener
./url-shortener
```

Expected output includes:

```text
All URL shortener checks passed
```

The executable contains an integration-style `main` that exercises generated codes, custom aliases, redirects, analytics, disabling, and invalid-input handling.

## Production mapping

The in-memory maps in this learning implementation correspond to the production architecture as follows:

| Reference implementation | Production component |
|---|---|
| `records_` | Durable URL-mapping store such as DynamoDB/Cassandra |
| `events_` | Kafka topic consumed by an analytics pipeline |
| `Base62CodeGenerator` | Snowflake/sequence service followed by Base62 encoding |
| `shared_mutex` | Stateless service replicas plus database/cache consistency controls |
| `UrlShortener::resolve` | Redirect service with Redis read-through cache |

A production API should add authentication, rate limiting, SSRF/malware screening, structured error responses, observability, and an idempotency key for create requests. Redirects should publish click events to a queue rather than retaining them in process memory.

See [`URLShortenerDesign.md`](URLShortenerDesign.md) for capacity estimates, APIs, schema, request flows, failure handling, and scalability decisions.
