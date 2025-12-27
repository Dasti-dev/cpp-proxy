#pragma once

/*
 * ConnectionState
 * ----------------
 * Explicit state machine for a single proxy connection.
 *
 * Rules:
 * - Every connection is always in exactly one state
 * - State transitions are explicit
 * - No implicit transitions
 * - Invalid transitions are logic bugs
 *
 * This enum is the backbone of correctness.
 */

enum class ConnectionState {
    /*
     * Connection object created but not yet registered.
     * Resources allocated, but no I/O possible yet.
     */
    INIT,

    /*
     * Client socket is open and waiting for data.
     * Interested in EPOLLIN.
     */
    CLIENT_READING,

    /*
     * Client request fully read.
     * Waiting to establish backend connection.
     */
    CONNECTING_BACKEND,

    /*
     * Backend socket connected.
     * Sending request to backend.
     */
    BACKEND_WRITING,

    /*
     * Waiting for backend response.
     */
    BACKEND_READING,

    /*
     * Sending backend response back to client.
     */
    CLIENT_WRITING,

    /*
     * Connection is shutting down.
     * Cleanup is in progress.
     */
    CLOSING,

    /*
     * Terminal state.
     * All resources released.
     */
    CLOSED
};
