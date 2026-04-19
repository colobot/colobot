"""Milestone 0 — /health endpoint."""


def test_health_ok(client):
    data = client.health()
    assert data["status"] == "ok"


def test_health_has_version(client):
    data = client.health()
    assert "version" in data
    assert data["version"]  # non-empty
