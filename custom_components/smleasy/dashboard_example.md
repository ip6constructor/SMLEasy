# SMLEasy – Home Assistant Dashboard (Kachelmuster)

Diese Vorlage baut ein Home-Assistant-Dashboard im selben "Kachel"-Stil wie die
Energy-Ansicht des Web-Dashboards nach: pro Verbraucher/Erzeuger eine Kachel,
darunter eine KPI-Leiste (Autarkie, Gesamterzeugung, Netzkosten, Ersparnis).

Es werden ausschliesslich Bordmittel von Home Assistant verwendet (`tile`- und
`grid`-Karten), keine zusaetzliche Custom-Card/HACS-Abhaengigkeit noetig.

## 1. Voraussetzung: Autarkie-Template-Sensor

Die SMLEasy-Integration liefert nur Rohwerte (Hausverbrauch, Netzbezug je
Zeitraum), keine fertige Autarkie-Prozentzahl – aus denselben Gruenden, aus
denen die Web-/App-Kennzahl "Eigenverbrauch" als Autarkiegrad
`(Hausverbrauch - Netzbezug) / Hausverbrauch` statt als PV-Eigenverbrauchsquote
berechnet wird (siehe `docs/ZIEL-DESIGN.md`, Fortschrittsnachweis 2026-10-29).
Dafuer ergaenzt dieser Template-Sensor die Integration, ohne sie zu aendern.

In `configuration.yaml` (oder ein Package) einfuegen und Home Assistant neu
starten bzw. "YAML neu laden -> Template-Entitaeten":

```yaml
template:
  - sensor:
      - name: "SMLEasy Autarkiegrad heute"
        unique_id: smleasy_autarkiegrad_heute
        unit_of_measurement: "%"
        icon: mdi:home-battery-outline
        state: >
          {% set haus = states('sensor.energie_smartmeter_smleasy_hausverbrauch_heute') | float(0) %}
          {% set bezug = states('sensor.energie_smartmeter_smleasy_bezug_heute') | float(0) %}
          {% if haus > 0.05 %}
            {{ ((haus - bezug) / haus * 100) | round(0) | max(0) | min(100) }}
          {% else %}
            0
          {% endif %}
```

**Wichtig:** Die Entity-IDs `sensor.energie_smartmeter_smleasy_hausverbrauch_heute`
und `sensor.energie_smartmeter_smleasy_bezug_heute` wurden am 29.10.2026 direkt
im laufenden HA-Testsystem (192.168.1.12) unter **Entwicklerwerkzeuge ->
Zustaende** verifiziert (Geraetename dort lautet "Energie Smartmeter", daher
das Praefix `energie_smartmeter_` vor `smleasy_...`). Auf einer anderen
HA-Instanz kann das Praefix je nach Geraetename abweichen – im Zweifel dort
nach "Hausverbrauch" bzw. "Bezug" suchen und die `states(...)`-Aufrufe
entsprechend anpassen.

## 2. Dashboard-YAML

Neues Dashboard anlegen (Einstellungen -> Dashboards -> Hinzufuegen -> "Leeres
Dashboard") und dort ueber das Drei-Punkte-Menue -> "Raw-Konfigurationseditor"
den Inhalt von [`dashboard.yaml`](dashboard.yaml) einfuegen. Die dort
referenzierten `sensor.goodwe_*`/`sensor.wallbox_*`/`sensor.evcc_*`-Entitaeten
entsprechen der aktuellen `ext`-Sensor-Zuordnung des Geraets
(`/api/config/ext`, Stand 2026-10-29) – bei einer anderen Belegung die
`entity:`-Zeilen entsprechend austauschen.
